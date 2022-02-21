set(groot, 'defaultAxesTickLabelInterpreter','latex'); set(groot, 'defaultLegendInterpreter','latex');
set(0,'defaultTextInterpreter','latex');
set(0,'defaultAxesFontSize',28);
figure(2)
clf;
% T=tiledlayout(4,2);
files = "../Output/Pollution/" + ["Active"] + "/StellarCatalogue.dat";

T = tiledlayout('flow');
for f = files
    g = readtable(f,"ReadVariableNames",true);
%     g(1:10,:)
%     scatter(

%     isolation = g.BirthRadius > 9 | g.BirthRadius < 6;
%     nI = ~isolation;
%     g(nI,:) = []; 

    nexttile;
    histogram2(g.Radius,g.BirthRadius,10,'FaceColor','flat');
    set(gca,'zscale','log');
    view(2) 
%     
%     nexttile;
%     histogram2(g.MeasuredAge, g.Radius - g.BirthRadius,20)
    
    nexttile;
    delta = g.MgH - g.FeH;
    cutter = ~((delta > 0.4) | (delta < -0.5) | (g.FeH < -5));
%     histogram2(g.FeH(cutter),delta(cutter),[20,30])

    spatialDelta = g.BirthRadius;
    n = height(g);
    xDelta = normrnd(0,0.05,n,1);
    yDelta = normrnd(0,0.02,n,1);
    scatter3(g.FeH,delta,g.BirthRadius,3,g.BirthRadius,'filled')
    colorbar
    x = g.FeH + xDelta;
    y = delta + yDelta;
    [N,X,Y] = histcounts2(x,y,100);
    N = N';
     Ncols = 10000;
    zeroed = hot(Ncols);
    zeroed(1,:) = [0,0,0];
    nexttile;
    colormap(zeroed)
    image([min(X),max(X)],[min(Y),max(Y)],N,'CDataMapping','scaled')
    set(gca,'YDir','normal')
    set(gca,'ColorScale','log')
    xlim([-1.4,0.6]);
    ylim([-0.2,0.4]);
    colorbar
    nexttile;
    histogram(delta(cutter),30)
%     set(gca,'yscale','log')
%     scatter(g.FeH,delta,1)
    
   
   
    nexttile;
    color = g.BMag - g.VMag;
    scatter(color,g.VMag,3,g.MeasuredAge,'filled');
%     histogram2(color(cutter),g.VMag(cutter),'FaceColor','flat');
    set(gca,'ydir','reverse');
    colorbar
    view(2);
%     set(gca,'yscale','log');
end
    