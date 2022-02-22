set(groot, 'defaultAxesTickLabelInterpreter','latex'); set(groot, 'defaultLegendInterpreter','latex');
set(0,'defaultTextInterpreter','latex');
set(0,'defaultAxesFontSize',28);


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
        cut = g.FeH < -10;
        g(cut,:) = [];
        disp("Cut")

        nexttile;
        histogram2(g.Radius,g.BirthRadius,15,'FaceColor','flat');
        set(gca,'zscale','log');
        view(2) 
    %     
    %     nexttile;
    %     histogram2(g.MeasuredAge, g.Radius - g.BirthRadius,20)

%         nexttile;
        delta = g.SiH - g.FeH;
        cutter = ~((delta > 0.6) | (delta < -0.5) | (g.FeH < -5));
    %     histogram2(g.FeH(cutter),delta(cutter),[20,30])

%         spatialDelta = g.BirthRadius;
        n = height(g);
        xDelta = normrnd(0,0.05,n,1);
        yDelta = normrnd(0,0.02,n,1);
%         scatter(g.FeH,delta,3,g.BirthRadius,'filled')
        
        disp("Plotted 1")
        Ncols = 100000;
        zeroed = jet(Ncols);
        zeroed(1,:) = [0,0,0];
        nexttile;



        colorbar
        x = g.FeH + xDelta;
        y = delta + yDelta;
        [N,X,Y] = histcounts2(x,y,100);
        N = N';


        colormap(zeroed)
        image([min(X),max(X)],[min(Y),max(Y)],N,'CDataMapping','scaled')
        clear x y N xDelta yDelta; 
        set(gca,'YDir','normal')
        set(gca,'ColorScale','log')
        disp("Plotted 2")
        xlim([-2.4,0.6]);
        ylim([-0.2,0.4]);
        colorbar
        nexttile;
        histogram(delta(cutter),30)
        set(gca,'yscale','log')
    %     scatter(g.FeH,delta,1)

    %    
    %    
    %     nexttile;
    %     color = g.BMag - g.VMag;
    %     scatter(color,g.VMag,3,g.MeasuredAge,'filled');
    %     histogram2(color(cutter),g.VMag(cutter),'FaceColor','flat');
    %     set(gca,'ydir','reverse');
    %     colorbar
    %     view(2);


%         nexttile;
%         histogram(g.Mass)
%         disp("Plotted 3")
%         set(gca,'yscale','log');

        thickSampler = (g.MeasuredAge < 3);
        z0 = 0.05;
        kappa = 0.3;
        pow = 0.66;
        
        thickScale = mean( z0 + kappa * g.MeasuredAge(thickSampler).^pow)
        thinScale = mean(z0 + kappa * g.MeasuredAge(~thickSampler).^pow)
    end
end
    