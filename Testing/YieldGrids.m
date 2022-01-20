ccsn = readtable("../Output/Test/CCSN_yields.dat");
agb = readtable("../Output/Test/AGB_yields.dat");
old = readtable("../Resources/ChemicalData/RalphSavedYields.dat");
column = "Mg";
oldColum = 3 + 5;
overplotter = "../Output/Test/" + column + "_ridges_CCSN.dat";
ridges = readtable(overplotter);
figure(3)
T = tiledlayout('flow');

xC = ccsn.Mass;
yC = ccsn.logZ;
zC = ccsn.(column);

nexttile;
% scatter3(xC,yC,zC);
tri = delaunay(xC,yC);
h = trisurf(tri, xC, yC, zC,'LineStyle','None');
hold on;
xA = agb.Mass;
yA = agb.logZ;
zA = agb.(column);
tri = delaunay(xA,yA);
h = trisurf(tri, xA, yA, zA);
% scatter3(ridges.Mass,ridges.logZ,ridges.Value,100,'r');
Zs = unique(ridges.logZ);
for z = Zs'
    z
   selector = (ridges.logZ ==z);
   plot3(ridges.Mass(selector),ones(sum(selector),1)*z,ridges.Value(selector),'LineWidth',5);
end
xO = old.Var1;
yO = log10(old.Var2);
zO = old.("Var" + num2str(oldColum));
scatter3(xO,yO,zO,100,'r','Filled');
hold off;