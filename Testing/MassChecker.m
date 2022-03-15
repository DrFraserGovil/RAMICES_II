set(groot, 'defaultAxesTickLabelInterpreter','latex'); set(groot, 'defaultLegendInterpreter','latex');
set(0,'defaultTextInterpreter','latex');
set(0,'defaultAxesFontSize',28);

files = "../Output/" + ["Calibration"] + "/Mass.dat";

for i = 1:length(files)
    figure(i);
    
    clf;
    plotMassFile(files(i));
    
end

function plotMassFile(fileName)
    f = readtable(fileName);

    time = unique(f.Time);
    radius = unique(f.Radius);

    clf;
    nSample = 150;
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
        hgm = f.HotGasMass(selector);
        sgm = f.StellarMass(selector);
        wdm = f.WDMass(selector);
        nsm = f.NSMass(selector);
%         igm(i) = mean(f.IGMMass(selector));

        mtotal(i) = sum(f.TotalMass(selector));
        stotal(i) = sum(sgm);
        ctotal(i) = sum(cgm);
        htotal(i) = sum(hgm);
        wdtotal(i) = sum(f.WDMass(selector));
        nstotal(i) = sum(f.NSMass(selector));
        bhtotal(i) = sum(f.BHMass(selector)) + stotal(i) + ctotal(i) + htotal(i) + wdtotal(i) + nstotal(i);

        nexttile(1);
        hold on;
        plot(rs,cgm./ f.SurfaceArea(selector),'Color',c(i,:));
        plot(rs,hgm./ f.SurfaceArea(selector),'Color',c(i,:),'LineStyle','--');
        hold off;
        nexttile(2);
        hold on;
        plot(rs,sgm./ f.SurfaceArea(selector),'Color',c(i,:));
        hold off;

        nexttile(3);
        hold on;
        plot(rs,hgm./cgm,'Color',c(i,:));
        hold off;

    end
    nexttile(1);
    xlabel("Central Radius of Ring (kpc)");
    ylabel("Gas ($10^9M_\odot $ kpc $^{-2}$)");
    set(gca,'yscale','log');
    ylim([1e-6,1])
    % legend(ts);
    grid on;
    caxis([0,t]);
    colorbar;

    nexttile(2);
    xlabel("Central Radius of Ring (kpc)");
    ylabel("Stars ($10^9M_\odot$kpc $^{-2}$)");
     
    trueTop = 25/1000;
    trueBottom = 60/1000;
    w = 0.5;
    sol = 8;
    hold on;
    fill([sol-w/2,sol-w/2,sol+w/2,sol+w/2],[trueBottom,trueTop,trueTop,trueBottom],[0.7,0.7,0.7],'FaceAlpha',0.7);
    ylim([1e-6,1])
    hold off;
    set(gca,'yscale','log');
    grid on;
    
    nexttile(3);
    xlabel("Central Radius of Ring (kpc)"); 
    ylabel("Hot-Cold Gas Ratio");
    set(gca,'yscale','log')
    ylim([1e-6,10])
    grid on;
    nexttile(4);
    
    lw = 2;
    plot(timeset,ctotal,'LineWidth',lw);
    hold on;
    
    plot(timeset,htotal,'LineWidth',lw);
    plot(timeset,stotal,'LineWidth',lw);
    plot(timeset,wdtotal,'LineWidth',lw);
    plot(timeset,nstotal,'LineWidth',lw);
    plot(timeset,bhtotal,'LineWidth',lw);
    plot(timeset,mtotal,'LineWidth',4);
    
%     plot(timeset,8 + 4.5*(1-exp(-timeset/0.3)) + 45 * (1 - exp(-timeset/14)),'k')
    
    % plot(timeset,ctotal+htotal+stotal)
    xlabel("Simulation Time (Gyr)");
    ylabel("Total Mass ($10^{9}M_\odot$)");
    legend("Cold Gas","Hot Gas", "Stars","White Dwarfs", "Neutron Stars", "Black Holes","Total mass");
%     set(gca,'yscale','log');
    grid on;
    title(T,"\verb|" + fileName + "|","Interpreter","latex");
end