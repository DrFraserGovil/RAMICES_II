import numpy as np
import pandas as pd
import warnings
from tqdm import tqdm
import glob
import sys
from concurrent.futures import ProcessPoolExecutor, as_completed
import gc

""" Python Script that matches isochrones to the population catalogue and weighs them according to the popolation mass."""

abbrev =  sys.argv[1] 

number_workers = 15



def find_closest_isochrone(row, padova, age_epsilon=1e-9, met_epsilon=1e-9):
    # Calculate age differences
    age_diff = (padova['logAge'] - row['LogAge']).abs()
    age_min = age_diff.min()


    # Keep all entries matching the minimal age difference (within a small epsilon if needed)
    age_subset = padova[age_diff <= age_min + age_epsilon]

    # Among those, find the minimal difference in metallicity
    met_diff = (age_subset['Zini'] - row['Metallicity']).abs()
    met_min = met_diff.min()

    # Keep all entries matching the minimal metallicity difference
    subset = age_subset[met_diff <= met_min + met_epsilon]

    # if (age_min < age_epsilon):
    if ((subset['Zini'].min()- subset['Zini'].max()) > 0 or (subset['Age'].min()- subset['Age'].max() > 0)):
        warnings.warn("Selected more than one isochrone")


    return subset


def weigh_isochrones(row, padova):
    
    isochrone = find_closest_isochrone(row, padova).copy()

    isochrone['weight'] = isochrone['IMF'] *row['PopulationMass']*1e9
    isochrone['Nstars'] = isochrone['weight']/isochrone['Mini']

    isochrone['Radius'] = row['Radius']
    isochrone['BirthRadius'] = row['BirthRadius']

    isochrone['PopulationMass'] = row['PopulationMass']
    isochrone['Metallicity'] = row['Metallicity']
    isochrone['HeH'] = row['HeH']
    isochrone['ZH'] = row['ZH']
    isochrone['FeH'] = row['FeH']
    isochrone['OH'] = row['OH']
    isochrone['MgH'] = row['MgH']
    isochrone['CH'] = row['CH']
    isochrone['SiH'] = row['SiH']
    isochrone['CaH'] = row['CaH']
    isochrone['MnH'] = row['MnH']
    isochrone['CrH'] = row['CrH']
    isochrone['CoH'] = row['CoH']
    isochrone['EuH'] = row['EuH']
    isochrone['LogAge'] = row['LogAge']


    return isochrone




ddf = pd.read_csv('./Output/' +abbrev+ '/PopulationCatalogue.dat', sep=', ')

ddf.drop(ddf.PopulationMass[ddf.PopulationMass == 0].index, inplace=True)

ddf['LogAge']  = np.log10(ddf['TrueAge']*1e9)




cols =["Zini","MH","logAge","Mini","int_IMF","Mass","logL","logTe","logg","label","McoreTP","C_O","period0","period1","period2","period3","period4","pmode","Mloss","tau1m","X","Y","Xc","Xn","Xo","Cexcess","Z","mbolmag","Umag","Bmag","Vmag","Rmag","Imag","Jmag","Hmag","Kmag"]

# Gather all isochorne .dat files
files_newpadova = sorted(glob.glob('./Resources/Isochrones/NewPadova/*.dat'))
files_padovafiles = sorted(glob.glob('./Resources/Isochrones/PadovaFiles/*.dat'))

# Load each file into a DataFrame and collect them in a list
dfs = []
for filename in files_newpadova + files_padovafiles:
    data = np.loadtxt(filename)
    df_temp = pd.DataFrame(data, columns=cols)
    dfs.append(df_temp)

# Concatenate all DataFrames into one
padova = pd.concat(dfs, ignore_index=True)

padova = padova.sort_values(by=['logAge', 'Zini']).reset_index(drop=True)


padova['Age'] = 10**padova['logAge'] / 1e9

# get the IMF from the integrated IMF
padova['IMF_diff'] = padova['int_IMF'].diff()
# Take care of the first entry in a new isochrone
padova['IMF'] = padova['IMF_diff'].where(padova['IMF_diff'] >= 0, padova['int_IMF'])

# For the very first row, diff() yields NaN, so also set it to its own int_IMF:
first_idx = padova.index[0]
padova.loc[first_idx, 'IMF'] = padova.loc[first_idx, 'int_IMF']

padova.drop(padova[padova['IMF'] == 0].index, inplace=True)


padova.drop([ 'McoreTP', 'C_O', 'period0', 'period1', 'period2',
    'period3', 'period4', 'pmode', 'Mloss', 'tau1m', 'X', 'Y', 'Xc', 'Xn',
    'Xo', 'Cexcess', 'IMF_diff'], inplace = True, axis = 1)

#drop post AGB isochrone points
padova.drop(padova[padova['label'] == 9].index, inplace=True)



## actually run the isochonre functions
rows = ddf.to_dict(orient='records')

def process_row(row_dict):
    row = pd.Series(row_dict)
    weighted_isochrones = weigh_isochrones(row, padova)
    return weighted_isochrones


with ProcessPoolExecutor(max_workers=number_workers) as executor:
    futures = [executor.submit(process_row, r) for r in rows]
    results = []
    for future in tqdm(as_completed(futures), total=len(futures), desc="Processing Populations"):
        results.append(future.result())


## writing the weighted isochrones to one file
for i, chunk in enumerate(tqdm(results, desc="Writing chunks to CSV", total=len(results))):
    df_chunk = chunk if isinstance(chunk, pd.DataFrame) else pd.DataFrame(chunk)
    if i == 0:
        df_chunk.to_csv("./Output/"+abbrev+"/WeightedPopulation.csv", mode="w", index=False, header=True)
    else:
        df_chunk.to_csv("./Output/"+abbrev+"/WeightedPopulation.csv", mode="a", index=False, header=False)


del results
del padova

gc.collect()