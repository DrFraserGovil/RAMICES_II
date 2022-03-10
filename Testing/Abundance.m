% files = "../Output/" + ["LowTempLowSpace","HighTempLowSpace","LowTempUniformLowSpace","HighTempUniformLowSpace","LowTempHighSpace","HighTempHighSpace","LowTempUniformHighSpace","HighTempUniformHighSpace"]+ "/Enrichment_Log_ColdGas.dat";
set(groot, 'defaultAxesTickLabelInterpreter','latex'); set(groot, 'defaultLegendInterpreter','latex');
set(0,'defaultTextInterpreter','latex');
set(0,'defaultAxesFontSize',28);
figure(3);
clf;
% T=tiledlayout(4,2);
files = "../Output/" + ["Calibration"] + "/Enrichment_Log_ColdGas.dat";

T = tiledlayout('flow');

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
% f(1:10,:)

fe = (f.Total_Fe);
h = f.Total_H;

mg = (f.Total_Mg);


% scatter(fe-h,mg-fe);
r = unique(f.RingRadius);
r = r(2:end-2);
% clf;
colormap(jet)
cs = jet(length(r));
for i = 1:length(r)
    selector = f.RingRadius == r(i);
  
    feSub = fe(selector);
    hSub = h(selector);
    mgSub = mg(selector);
    
    feH = feSub - hSub;
    if i == 40
        [min(feH), max(feH)]
    end
    delta = f.Total_Mg(selector) - f.Total_Fe(selector);
    
    hold on;
%     plot(f.TimeIndex(selector)*0.02, mgSub- hSub,'Color',cs(i,:));
%     set(gca,'xscale','log')
    plot(feH, delta, 'Color',cs(i,:));
   
end
 colorbar;
 caxis([min(r),max(r)])
 xlim([-2.5,1.2]);
%  ylim([-0.3,0.5]);
 hold off;
    
title("\verb|" +fileName + "|","FontSize",10);
end
