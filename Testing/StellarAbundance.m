set(groot, 'defaultAxesTickLabelInterpreter','latex'); set(groot, 'defaultLegendInterpreter','latex');
set(0,'defaultTextInterpreter','latex');
set(0,'defaultAxesFontSize',20);


% T=tiledlayout(4,2);
files = "../Output/Pollution/" + ["Active"] + "/StellarCatalogue.dat";

plotter(files,1)
function plotter(files,i)
    q = figure(i);
    clf;
    T = tiledlayout('flow');
    for f = files
        g = readtable(f,"ReadVariableNames",true);
        disp("Loaded")
        cut = g.FeH < -10 ;
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
        
        xDelta = normrnd(0,1,n,1) .*  0.05;
        yDelta = normrnd(0,1,n,1) .* 0.05;    ;
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
        histogram(y,30)
        xlabel("[Mg/Fe]");
        ylabel("Counts");
%         set(gca,'yscale','log')
%           clear x y N xDelta yDelta; 
    %     scatter(g.FeH,delta,1)

    %    
    %    
        nexttile;
        color = g.BMag - g.VMag;
        histogram2(color(cutter),g.VMag(cutter),200,'FaceColor','flat',"ShowEmptyBins",true);
        set(gca,'ydir','reverse');
        set(gca,'ColorScale','log')
        xlabel("$M_B - M_V$");
        ylabel("$M_V$");
        colorbar
        view(2);


%         nexttile;
%         histogram(g.Mass)
%         disp("Plotted 3")
%         set(gca,'yscale','log');

        thickSampler = (g.MeasuredAge < 3);
        z0 = 0.02;
        kappa = 0.15;
        pow = 0.66;
        
        thickScale = mean( z0 + kappa * g.MeasuredAge(thickSampler).^pow)
        thinScale = mean(z0 + kappa * g.MeasuredAge(~thickSampler).^pow)
        
        
         nexttile;
        histogram2(g.MeasuredAge, g.Radius - g.BirthRadius,20,'FaceColor','flat',"ShowEmptyBins",true)
%         set(gca,'zscale','log');
        set(gca,'colorscale','log')
        xlabel("Age (Gyr)");
        ylabel("Outward Migration (kpc)");
        view(2) 
        
        
%         nexttile;
%         m= max(g.MeasuredAge);
%         scatter(m - g.MeasuredAge, g.MgH,5,g.TEff);
%         colorbar
%         nexttile(1);
%         sampleCut =  (g.MgH > (0.8 *  g.FeH+ 0.1)) & (g.FeH < -0.6);
%         hold on
%         scatter(x(sampleCut),y(sampleCut));
%         hold off;
%         
%         nexttile(3);
%         hold on;
%         scatter(color(sampleCut),g.VMag(sampleCut),100,'b','filled');
%         hold off;
%         
%      
%      
%         q = g(sampleCut,:);
%         q(1:100,:)
    end
end
    