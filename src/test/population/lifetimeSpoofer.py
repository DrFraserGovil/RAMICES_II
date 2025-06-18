import numpy as np
import matplotlib.pyplot as pt
import tarfile
import io
import sys
def tau(M,logZ):
	Mc = 2.7 + logZ/20
	J = 0.3 - logZ/10
	tSol = 10
	mSol = 1
	decay = 1 - logZ/10
	decay2 = 0.1 - logZ/200
	w = 0.60001 + logZ/10

	mode1 = lambda ms: tSol * np.exp(-decay*(ms-mSol)) * (ms <=Mc)
	mode2 = lambda ms: (mode1(Mc) + J*(ms-Mc)/w)*np.logical_and(ms>Mc,ms<=Mc+w)
	mode3 = lambda ms: (mode2(Mc+w)* np.exp(-decay2*(ms-Mc-w))) * (ms>(Mc+w))

	return mode1(M)+ mode2(M) + mode3(M)

def massSample(lowRes=40,highstep=5):
	return np.concat([np.linspace(0.07,3.4,lowRes),np.arange(3.5,10,0.5),np.arange(10,120,highstep)])

def generateFiles(name):
	
	manifestInfo = []
	with tarfile.open(name,'w|') as tar:
		idx = 0
		for lz in range(-6,0):
			lowRes = int(7e1 + abs(lz))
			highStep = int(3 +abs(lz)/2)
			ms = massSample(lowRes,highStep)

			fname = f"lifetime_{idx}.dat" 
			manifestInfo.append(f"{lz} {fname}")
			ts = tau(ms,lz)
			content = "\n".join([f"{ms[i]} {t}" for i,t in enumerate(ts)])
			
			file_data = content.encode('utf-8') # Tarfile prefers bytes
			info_stream = io.BytesIO(file_data)
			tar_info = tarfile.TarInfo(name=fname)
			tar_info.size = len(file_data) # Set the size of the content
			tar.addfile(tar_info,info_stream)
			idx += 1
		manifest= "\n".join(manifestInfo)
		file_data = manifest.encode('utf-8') # Tarfile prefers bytes
		info_stream = io.BytesIO(file_data)
		tar_info = tarfile.TarInfo(name="manifest.dat")
		tar_info.size = len(file_data) # Set the size of the content
		tar.addfile(tar_info,info_stream)
def plot():

	for lz in range(-6,-5):
		lowRes = 40 + abs(lz)
		highStep = int(3 +abs(lz)/2)
		ms = massSample(lowRes,highStep)
		ts = tau(ms,lz)
		pt.plot(ms,ts,label=f"log(Z)={lz}")
		pt.scatter(ms,ts,10,marker="x")

	for r in range(1000):
		y = r*0.01
		pt.plot([0,120],[y,y],alpha=0.5,color='gray')
	# pt.yscale('log')
	# pt.xscale('log')
	# pt.xscale('symlog',linthresh=1)
	
	pt.legend()

	pt.draw()
	pt.pause(0.1)
	input("enter to exit")
if __name__ == "__main__":
	name = "resources/Isochrones/unittest.iso"
	if (len(sys.argv) > 1):
		name = sys.argv[1]

	if name == "plot":
		plot()
	else:
		generateFiles(name)

