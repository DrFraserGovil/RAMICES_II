% files = "../Output/" + ["LowTempLowSpace","HighTempLowSpace","LowTempUniformLowSpace","HighTempUniformLowSpace","LowTempHighSpace","HighTempHighSpace","LowTempUniformHighSpace","HighTempUniformHighSpace"]+ "/Enrichment_Log_ColdGas.dat";
set(groot, 'defaultAxesTickLabelInterpreter','latex'); set(groot, 'defaultLegendInterpreter','latex');
set(0,'defaultTextInterpreter','latex');
set(0,'defaultAxesFontSize',28);
clf;
% T=tiledlayout(4,2);
files = "../Output/Test_" + ["SLF"] +"/Enrichment_Log_ColdGas.dat";
T = tiledlayout(1,1);
for file = files
    nexttile;
    file
    plotter(file);
    grid on;
end
xlabel(T,"[Fe/H]");
ylabel(T,"[Mg/Fe]");

function plotter(fileName)
opts = detectImportOptions(fileName);

opts.VariableTypes(:) = {'double'};

f = readtable(fileName,opts);

fe = (f.Total_Fe);
h = f.Total_H;

mg = (f.Total_Mg);


% scatter(fe-h,mg-fe);
r = unique(f.RingIndex);
% clf;
colormap(jet)
cs = jet(length(r));
for i = 1:length(r)
    selector = f.RingIndex == r(i);
  
    feSub = fe(selector);
    hSub = h(selector);
    mgSub = mg(selector);
    
    
    hold on;
%     plot(f.TimeIndex(selector)*0.02, mgSub- hSub,'Color',cs(i,:));
%     set(gca,'xscale','log')
    plot(feSub - hSub, mgSub - feSub, 'Color',cs(i,:));
    colorbar;
    caxis([min(r),max(r)])
    xlim([-3,0.5]);
%     ylim([0,0.35]);
    hold off;
    
end
title("\verb|" +fileName + "|","FontSize",10);
end
