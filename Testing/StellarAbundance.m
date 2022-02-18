set(groot, 'defaultAxesTickLabelInterpreter','latex'); set(groot, 'defaultLegendInterpreter','latex');
set(0,'defaultTextInterpreter','latex');
set(0,'defaultAxesFontSize',28);
figure(2)
clf;
% T=tiledlayout(4,2);
files = "../Output/" + ["SynthesisTest"] + "/StellarCatalogue.dat";

T = tiledlayout('flow');
for f = files
    g = readtable(f,"ReadVariableNames",true);
%     g(1:10,:)
%     scatter(

%     nexttile;
%     histogram2(g.Radius,g.BirthRadius,20);
%     
%     nexttile;
%     histogram2(g.MeasuredAge, g.Radius - g.BirthRadius,20)
    
    nexttile;
    delta = g.MgH - g.FeH;
    cutter = ~((delta > 0.4) | (delta < -0.5) | (g.FeH < -3));
%     histogram2(g.FeH(cutter),delta(cutter),[20,30])
    scatter(g.FeH,delta,1,'filled')
    xlim([-4,1]);
    ylim([-0.3,0.5]);
    nexttile;
    histogram(delta(cutter),30)
%     set(gca,'yscale','log')
%     scatter(g.FeH,delta,1)
    
   
   
    nexttile;
    color = g.BMag - g.VMag;
    ts= unique(g.MeasuredAge);
    for i = 1:length(ts)
        ts(i)
        cutter = (g.MeasuredAge == ts(i));
        hold on;
        scatter(color(cutter),g.VMag(cutter),5,'filled');
        hold off;
    end
%     histogram2(color(cutter),g.VMag(cutter),'FaceColor','flat');
    set(gca,'ydir','reverse');
    view(2);
%     set(gca,'yscale','log');
end
    