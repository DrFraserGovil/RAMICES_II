set(groot, 'defaultAxesTickLabelInterpreter','latex'); set(groot, 'defaultLegendInterpreter','latex');
set(0,'defaultTextInterpreter','latex');
set(0,'defaultAxesFontSize',28);

f = readtable("../Output/Galactic/Mass.dat");

time = unique(f.Time);
radius = unique(f.Radius);

clf;
nSample = 300;
ts = string.empty;
timeset = time(ceil(linspace(1,length(time),nSample)))';
mset = zeros(size(timeset));
accActual = mset;
accPred = mset;
igm = mset;
mtotal = mset;
stotal = mset;
ctotal = mset;
htotal= mset;
T = tiledlayout(2,2,"TileSpacing","Compact","Padding","Compact");
c= (jet(nSample));
colormap(jet);
for i = 1:nSample
    t = timeset(i);
    ts(end+1) = num2str(t);
    
    selector = (f.Time == t);
    rs = f.Radius(selector);
    cgm = f.ColdGasMass(selector);
    sgm = f.StellarMass(selector);
    igm(i) = mean(f.IGMMass(selector));
    
    mtotal(i) = sum(f.TotalMass(selector));
    stotal(i) = sum(sgm);
    ctotal(i) = sum(cgm);
    htotal(i) = sum(f.HotGasMass(selector));
    
    nexttile(1);
    hold on;
    plot(rs,cgm,'Color',c(i,:));
    hold off;
    nexttile(2);
    hold on;
    plot(rs,sgm,'Color',c(i,:));
    hold off;
    
    nexttile(3);
    hold on;
    plot(rs,cgm./sgm,'Color',c(i,:));
    hold off;
    
end
nexttile(1);
xlabel("Central Radius of Ring (kpc)");
ylabel("Gas in Ring ($10^9M_\odot$)");
% legend(ts);
caxis([0,t]);
colorbar;

nexttile(2);
xlabel("Central Radius of Ring (kpc)");
ylabel("Stars in Ring ($10^9M_\odot$)");

nexttile(3);
xlabel("Central Radius of Ring (kpc)");
ylabel("Cold-Stellar Mass Ratio");
set(gca,'yscale','log')

nexttile(4);
plot(timeset,ctotal);
hold on;

plot(timeset,htotal);
plot(timeset,stotal);
plot(timeset,mtotal,'LineWidth',2);
% plot(timeset,ctotal+htotal+stotal)
xlabel("Simulation Time (Gyr)");
ylabel("Total Mass");
legend("Cold Gas","Hot Gas", "Stars","Total mass");
