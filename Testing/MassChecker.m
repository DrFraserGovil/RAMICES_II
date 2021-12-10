set(groot, 'defaultAxesTickLabelInterpreter','latex'); set(groot, 'defaultLegendInterpreter','latex');
set(0,'defaultTextInterpreter','latex');
set(0,'defaultAxesFontSize',28);

f = readtable("../Output/Galactic/Mass.dat");

time = unique(f.Time);
radius = unique(f.Radius);

clf;
nSample = 30;
ts = string.empty;
timeset = time(ceil(linspace(1,length(time),nSample)))';
mset = zeros(size(timeset));
accActual = mset;
accPred = mset;
igm = mset;
T = tiledlayout(2,1,"TileSpacing","Compact","Padding","Compact");

for i = 1:nSample
    t = timeset(i);
    ts(end+1) = num2str(t);
    
    selector = (f.Time == t);
    rs = f.Radius(selector);
    cgm = f.ColdGasMass(selector);
    igm(i) = mean(f.IGMMass(selector));
    mset(i) = sum(f.TotalMass(selector));
  

    nexttile(1);
    hold on;
    plot(rs,cgm);
    hold off;
    nexttile(2);
    
    
    
end
nexttile(1);
xlabel("Central Radius of Ring (kpc)");
ylabel("Mass in Ring ($10^9M_\odot$)");
legend(ts);

nexttile(2);
plot(timeset,mset);
xlabel("Simulation Time (Gyr)");
ylabel("Total Galaxy Mass");

