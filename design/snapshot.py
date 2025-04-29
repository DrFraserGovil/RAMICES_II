import numpy as np

import matplotlib.pyplot as plt
import matplotlib.animation as animation
np.set_printoptions(threshold=np.inf)
np.set_printoptions(linewidth=np.inf)

Mc = 1.7
beta = 2.2
gamma = beta
Mmax = 100
def tau(m):
	tauc = 10.0*np.exp(-beta*(Mc - 1))
	branch1 = tauc * np.exp(-beta*(m - Mc))
	branch2 = tauc/(gamma * np.abs(m - Mc) + 1)
	return branch1 *(m<=Mc) + branch2 *(m>Mc)	

def zeta(m):
	alpha = 2.3
	a = (alpha - 1)/alpha
	Mc = 1
	return a * ( (m <= Mc) + (m>Mc)*np.power(m+1e-90,-alpha))


def S(t,Delta,delta):
	b = 0
	b +=  (t - Delta)/delta * (t>Delta)*(t < Delta + delta)
	b += (t >= Delta + delta)
	return b

def GetIsochrones(N=10):
	return np.exp(np.linspace(-3,np.log(Mmax),N))
	bins = np.linspace(0,Mmax,1000)
	prob = zeta(bins)
	prob/=np.trapezoid(prob,bins)
	prob/=np.sum(prob)
	masses = []
	cumprob = 0
	target = 0

	Nprop = int(np.ceil(0.6*N))
	dcum = 1.0/(Nprop-1)
	for i,p in enumerate(prob):
		cumprob += p
		if cumprob > target or np.abs(cumprob-target) < 1e-5:
			print(f"{cumprob} is greater than {target}")
			target += dcum
			masses.append(bins[i])
	
	Nlin = N - Nprop
	masses = np.sort(np.concat([masses,np.linspace(0.07,99,Nlin)]))
	print(masses,len(masses))
	# masses = np.array(masses)
	return masses

class TauInterpolator:
	def __init__(self,masses):
		self.x = masses
		self.y = tau(masses)

	def __call__(self,mass):
		grab = np.searchsorted(self.x,mass,side='right')-1
		interp = (mass - self.x[grab])/(self.x[grab+1] - self.x[grab-1])
		return self.y[grab] + interp * (self.y[grab+1] - self.y[grab])

def BruteForce(times,dt,bins,samples=None):
	binsizes = np.insert(np.diff(bins),-1,Mmax-bins[-1])

	pop = zeta(bins)
	pop /=np.trapezoid(pop,bins)
	if samples is not None:
		getTau = TauInterpolator(samples)
	else:
		getTau = tau
	lifetimes = getTau(bins)
	out = []
	for time in times:
		pop[lifetimes < time+dt] = 0
		out.append(pop.copy())
		# axs[ax].bar(bins,pop,0.5*(bins[1]-bins[0]))
	return out

def Analytical(times,dt,bins,samples=None):
	pop = zeta(bins)
	pop/=np.trapezoid(pop,bins)
	if samples is not None:
		getTau = TauInterpolator(samples)
	else:
		getTau = tau
	lifetimes = getTau(bins)
	out = []
	for time in times:
		s = S(lifetimes,time,dt)
		out.append(s*pop)
	return out

def Q(x,Delta,delta):
	b = 0
	b += (x-Delta)**2/(2*delta) * (x >= Delta)*(x<Delta + delta)
	b += (x - Delta - delta/2) * (x>=Delta + delta)
	return b

def Gridded(times,dt,sample,bins):
	pop = zeta(bins)
	pop/=np.trapezoid(pop,bins)
	sampleLife = tau(sample)
	a_ray = np.searchsorted(sample,bins,side='right')-1
	
	ta = sampleLife[a_ray]
	taP = sampleLife[a_ray+1]

	taum = np.minimum(ta,taP)
	taup = np.maximum(ta,taP)

	# for i,m in enumerate(bins):
	# 	print(f"{sample[a_ray[i]]} < {m} < {sample[a_ray[i]+1]} --> {taum[i]} < tau < {taup[i]}")

	diff = taup - taum
	out = []
	for time in times:
		s = (Q(taup,time,dt) - Q(taum,time,dt))/diff
		out.append(s*pop)
	return out

def configureAxes(t):
	fig.suptitle(f"t={t}")
	ax = 0,0
	axs[ax].cla()
	axs[ax].set_xscale('log')
	axs[ax].set_yscale('log')
	
def sim(n_iso):
	global fig,axs
	fig, axs = plt.subplots(2,1,squeeze=False,figsize=(15,10))
	simdt = 0.01
	plotBins = np.exp(np.linspace(-3,np.log(Mmax-1),1000))
	pop0 = zeta(plotBins)
	pop0 /= np.trapezoid(pop0,plotBins)
	
	sample = GetIsochrones(n_iso)

	# ts = np.exp(np.linspace(-3,np.log(35),500))
	join = 3
	# ts = np.concat([np.exp(np.linspace(np.log(simdt),np.log(join),200)),np.linspace(join,15,200)])
	ts = np.arange(0,15,simdt)
	ts = simdt*np.unique(np.round(np.concat([np.logspace(0,np.log10(10/simdt),300)-1,np.linspace(10/simdt,15/simdt,100)])))
	print(ts)
	print(len(ts))
	bruteSets = [np.linspace(0,99,201)]
	bruteEdges = []
	brutes = []
	for bruteBins in bruteSets:

		bruteEdges.append(np.insert(bruteBins,-1,bruteBins[-1]+4))
		brutes.append(np.array(BruteForce(ts,simdt,bruteBins,sample)))
	
	trueSims = np.array(Analytical(ts,simdt,plotBins))
	interpSims = Analytical(ts,simdt,plotBins,sample)
	gridSims = np.array(Gridded(ts,simdt,sample,plotBins))
	
	ax = 0,0
	axs[ax].set_ylim(bottom=1e-4,top=3.5*pop0[0])
	axs[ax].set_xscale('log')
	axs[ax].set_yscale('log')
	axs[ax].set_xlabel("Mass ($M_\\odot$)")
	axs[ax].set_ylabel("Fraction of Population")
	trueLine = axs[ax].plot(plotBins,trueSims[0],'k',label="Ground Truth")[0]
	binLines = []
	for j,bins in enumerate(bruteSets):
		# binLines.append(axs[ax].stairs(brutes[j][0],bruteEdges)[0])
		binLines.append(axs[ax].stairs(brutes[j][0],bruteEdges[j],label=f"Binned {len(bins)}"))
	interpLine =axs[ax].plot(plotBins,interpSims[0],label="Linear Interpolation")[0]
	gridLine = axs[ax].plot(plotBins,gridSims[0],label="Uniform Distribution")[0]
	axs[ax].legend()
	ax = 1,0
	axs[ax].set_xscale('log')
	# axs[ax].set_yscale('symlog')
	axs[ax].set_ylim([-0.5,0.5])
	axs[ax].set_xlabel("Mass ($M_\\odot$)")
	axs[ax].set_ylabel("Deviation From Truth")
	binDev = []
	for j,bins in enumerate(bruteSets):
		binDev.append(axs[ax].stairs(brutes[j][0]*np.nan,bruteEdges[j],label=f"Binned {len(bins)}"))
	interpDev =axs[ax].plot(plotBins,(interpSims[0]-trueSims[0])/(trueSims[0]+1e-6))[0]
	gridDev = axs[ax].plot(plotBins,(gridSims[0]-trueSims[0])/(trueSims[0]+1e-6),label="Uniform Distribution")[0]

	
	def updateFrame(frame):
		truth = trueSims[frame]
		truth/=np.trapezoid(truth,plotBins)
		trueLine.set_ydata(truth)
		interp = interpSims[frame]
		interp /= np.trapezoid(interp,plotBins)
		interpLine.set_ydata(interp)
		for j in range(len(binLines)):
			bin = brutes[j][frame]
			bin /= np.trapezoid(bin,bruteSets[j])
			binLines[j].set_data(bin)
		grid = gridSims[frame]
		grid/= np.trapezoid(grid,plotBins)
		gridLine.set_ydata(grid)
		fig.suptitle(f"Population Age: {ts[frame]:.3g}Gyr")

		interpDev.set_ydata((interp-truth)/(truth+1e-6))
		gridDev.set_ydata((grid-truth)/(truth+1e-6))

	def updateFrameCumulative(frame):

		truth = np.sum(trueSims[:frame+1],0)
		truth/=np.trapezoid(truth,plotBins)
		trueLine.set_ydata(truth)
		interp = np.sum(interpSims[:frame+1],0)
		interp /= np.trapezoid(interp,plotBins)
		interpLine.set_ydata(interp)
		for j in range(len(binLines)):
			bin = np.sum(brutes[j][:frame+1],0)
			bin /= np.trapezoid(bin,bruteSets[j])
			binLines[j].set_data(bin)
		grid = np.sum(gridSims[:frame+1],0)
		grid/= np.trapezoid(grid,plotBins)
		gridLine.set_ydata(grid)
		fig.suptitle(f"Galaxy Age {ts[frame]:.3g}Gyr")

		interpDev.set_ydata((interp-truth)/truth)
		gridDev.set_ydata((grid-truth)/truth)
		
	mode = 1
	if mode == 0:
		ani = animation.FuncAnimation(fig=fig,func=updateFrame,frames=len(trueSims),interval=30)
		plt.show()
	else:
		ani1 = animation.FuncAnimation(fig=fig,func=updateFrame,frames=len(trueSims),interval=30)
		ani1.save(f"single_{n_iso}_isochrones.mp4",writer="ffmpeg")
		ani2 = animation.FuncAnimation(fig=fig,func=updateFrameCumulative,frames=len(trueSims),interval=30)
		ani2.save(f"population_{n_iso}_isochrones.mp4",writer="ffmpeg")


def main():
	for iso in [10,200]:
		sim(iso)
if __name__ == "__main__":
	main()