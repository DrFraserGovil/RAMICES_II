dir = "../Output/Calibration/Yields/";
ccsn = readtable(dir + "CCSN_yields.dat");
agb = readtable(dir + "AGB_yields.dat");
% ecsn = readtable(dir + "ECSN_yields.dat");
old = readtable("../Resources/ChemicalData/RalphSavedYields.dat");
column = "O";
oldColum = 3 + 4;
overplotter = dir + column + "_ridges_CCSN.dat";
if column == "Remnant"
    column = column + "Fraction";
end
ridges = readtable(overplotter);
figure(3)
T = tiledlayout('flow');

xC = ccsn.Mass;
xC(xC == max(xC)) = 100;
cut = xC > 0.01;
xC = xC(cut);
yC = 10.^ccsn.logZ(cut);
zC = ccsn.(column)(cut);

nexttile;
% scatter3(xC,yC,zC);
tri = delaunay(xC,yC);
h = trisurf(tri, xC, yC, zC,'LineStyle','None');
hold on;
xA = agb.Mass;
yA = agb.logZ;
zA = agb.(column);
tri = delaunay(xA,yA);
% h = trisurf(tri, xA, yA, zA,'LineStyle','None');

% xE = ecsn.Mass;
% yE = ecsn.logZ;
% zE = ecsn.(column);
% % scatter3(xC,yC,zC);
% tri = delaunay(xE,yE);
% h = trisurf(tri, xE, yE, zE,'LineStyle','None');



% scatter3(ridges.Mass,ridges.logZ,ridges.Value,100,'r');
Zs = unique(ridges.logZ);
for z = Zs'
    z
   selector = (ridges.logZ ==z);
   m = ridges.Mass(selector);
   y = ridges.Value(selector);
   [~,I] = sort(ridges.Mass(selector));
   
   plot3(m(I),ones(sum(selector),1)*10.^z,y(I),'LineWidth',5);
end
xO = old.Var1;
yO = (old.Var2);
zO = old.("Var" + num2str(oldColum));
scatter3(xO,yO,zO,10,'r','Filled');
hold off;
view(2);
set(gca,'xscale','log');
set(gca,'yscale','log');
ylabel("Metallicity $Z$");
xlabel("Stellar Mass ($M_\odot$)");
c = colorbar;