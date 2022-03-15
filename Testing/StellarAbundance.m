set(groot, 'defaultAxesTickLabelInterpreter','latex'); set(groot, 'defaultLegendInterpreter','latex');
set(0,'defaultTextInterpreter','latex');
set(0,'defaultAxesFontSize',20);


% T=tiledlayout(4,2);
files = "../Output/" + ["Calibration"] + "/StellarCatalogue.dat";

plotter(files,1)
function plotter(files,i)
    q = figure(i);
    clf;
    T = tiledlayout('flow');
    for f = files
        g = readtable(f,"ReadVariableNames",true);
        g(1,:)
        disp("Loaded")
        cut = g.FeH < -2.5 | (g.EuH - g.FeH < -1) | g.MeasuredAge < 0.1 ;
        g(cut,:) = [];
        disp("Cut")
        
        mean(10.^g.TEff)

    %     
%        
        
        
%         nexttile;
        delta = g.MgH - g.FeH;
        cutter = ~((delta > 0.6) | (delta < -0.5) | (g.FeH < -5));
    %     histogram2(g.FeH(cutter),delta(cutter),[20,30])

%         spatialDelta = g.BirthRadius;
        n = height(g);
        
%         scaling = 
        
        xDelta = normrnd(0,1,n,1) .*  0.04;
        yDelta = normrnd(0,1,n,1) .* 0.0;   
%         scatter(g.FeH,delta,3,g.BirthRadius,'filled')
        
        disp("Plotted 1")
        Ncols = 100000;
        zeroed = hot(Ncols);
        zeroed(1,:) = [0,0,0];
        nexttile;

        
        colorbar
        x = g.FeH + xDelta;
        y = delta + yDelta;
        [N,X,Y] = histcounts2(x,y,300);
        N = N';


        colormap(zeroed)
        image([min(X),max(X)],[min(Y),max(Y)],N,'CDataMapping','scaled')
        
        set(gca,'YDir','normal')
        set(gca,'ColorScale','log')
        disp("Plotted 2")
        xlim([-2.4,0.6]);
        xlabel("[Fe/H]");
        ylabel("[Mg/Fe]");
%         ylim([-0.2,0.4]);
        colorbar
        grid on;

        nexttile;
        histogram(y,300,'LineStyle','None')
        xlabel("[Mg/Fe]");
        ylabel("Counts");
        set(gca,'yscale','log');
%         set(gca,'yscale','log')
%           clear x y N xDelta yDelta; 
    %     scatter(g.FeH,delta,1)

    %    
    %    
        nexttile;
        color = g.JMag - g.KMag;
        n = length(color(cutter));
        xDelta = normrnd(0,1,n,1) .*  0.0;
        yDelta = normrnd(0,1,n,1) .* 0.05;   
        histogram2(color(cutter)+xDelta,g.KMag(cutter)+yDelta,80,'FaceColor','flat',"ShowEmptyBins",true);
        set(gca,'ydir','reverse');
        set(gca,'ColorScale','log')
        xlabel("$M_B - M_V$");
        ylabel("$M_V$");
        colorbar
        view(2);

        
        thickSampler = (g.MeasuredAge < 3);
        z0 = 0.1;
        kappa = 0.3;
        pow = 0.66;
        
        thickScale = mean( z0 + kappa * g.MeasuredAge(thickSampler).^pow)
        thinScale = mean(z0 + kappa * g.MeasuredAge(~thickSampler).^pow)
        
        spatialDelta = g.BirthRadius;
        n = length(spatialDelta);
        deltaR1 = rand([n,1])*0.2 - 0.1;
        deltaR2 = rand([n,1])*0.2 - 0.1;
        
        deltadelta = deltaR1 - deltaR2;
        mean(deltadelta)
        spatialDelta = spatialDelta + deltadelta;
        bR = g.BirthRadius + deltaR1;
        cR = g.Radius + deltaR2;
        figure(2);
        clf;
        T = tiledlayout('flow');
        colormap(hot)
        spatialPlot(g.MeasuredAge,bR,"Age (Gyr)","Birth Radius (kpc)");
        spatialPlot(g.MeasuredAge,cR,"Age (Gyr)","Current Radius (kpc)");
        spatialPlot(g.MeasuredAge, cR - bR,"Age (Gyr)", "Outward Migration (kpc)");
        spatialPlot(bR, cR - bR,"Birth Radius (kpc)", "Outward Migration (kpc)");
        colorbar
    end
end
function spatialPlot(age,spatialData,xL,yL)
     nexttile;
     histogram2(age,spatialData,30,'FaceColor','flat',"ShowEmptyBins",true);
%         set(gca,'zscale','log')
         set(gca,'ColorScale','log')
        view(2)
    xlabel(xL);
    ylabel(yL);
end