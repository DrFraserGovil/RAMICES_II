    set(groot, 'defaultAxesTickLabelInterpreter','latex'); set(groot, 'defaultLegendInterpreter','latex');
set(0,'defaultTextInterpreter','latex');
set(0,'defaultAxesFontSize',28);

files = "../Output/" + ["Pollution/Active"] + "/Events.dat";


for i = 1:length(files)
    figure(i);
    plotEventFile(files(i));
    
end

function plotEventFile(fileName)
    f = readtable(fileName);
%     f.Properties.VariableNames = ["Time", "Radius", "StarMassFormed", "StarsFormed", "CCSN_Events", "AGB_Deaths", "NSM_Events", "SNIa_Events", "ECSN_Events", "BirthRate", "CCSNRate", "AGBRate", "NSMRate", "SNIaRate"];
    f(1:10,:)
    time = unique(f.Time);
    radius = unique(f.Radius);

    clf;
    nSample = 200;
    ts = string.empty;
    timeset = time(ceil(linspace(1,length(time),nSample)))';
    mset = zeros(size(timeset));
   
    totalBirth = mset;
    totalCCSN = mset;
    totalAGB = mset;
    totalNSM = mset;
    totalSNIa = mset;
    totalECSN = mset;
    T = tiledlayout(4,2,"TileSpacing","Compact","Padding","Compact");
    c= (jet(nSample));
    colormap(jet);
    for i = 1:nSample
        t = timeset(i);
        ts(end+1) = num2str(t);

        selector = (f.Time == t);
        rs = f.Radius(selector);
       
        birth = f.StarsFormed(selector);
        ccsn = f.CCSN_Events(selector);
        agb = f.AGB_Deaths(selector);
        nsm = f.NSM_Events(selector);
        snia = f.ECSN_Events(selector);
        
        totalBirth(i) = sum(f.BirthRate(selector));
        totalCCSN(i) = sum(f.CCSNRate(selector));
        totalAGB(i) = sum(f.AGBRate(selector));
        totalNSM(i) = sum(f.NSMRate(selector));
        totalSNIa(i) = sum(f.SNIaRate(selector));
        totalECSN(i) = sum(f.ECSNRate(selector));
        
%         divider = f.SurfaceArea(selector);
        nexttile(1);
        hold on;
        plot(rs,birth,'Color',c(i,:));
        hold off;
        nexttile(2);
        hold on;
        plot(rs,ccsn,'Color',c(i,:));
        hold off;

        nexttile(3);
        hold on;
        plot(rs,agb,'Color',c(i,:));
        hold off;
        
        nexttile(4);
        hold on;
        plot(rs,snia./ccsn,'Color',c(i,:));
        hold off;
% 
%         nexttile(5);
%         hold on;
%         plot(rs,nsm,'Color',c(i,:));
%         hold off;
    end
    nexttile(1);
%     xlabel("Central Radius of Ring (kpc)");
    ylabel("Birth Count");
    set(gca,'yscale','log');
    % legend(ts);
    caxis([0,t]);
    colorbar;
    grid on;
    
    nexttile(2);
%     xlabel("Central Radius of Ring (kpc)");
    ylabel("CCSN Count");
    set(gca,'yscale','log');
    grid on;
%     ylim([10^4,1r0^6])
    
    nexttile(3);
    xlabel("Central Radius of Ring (kpc)"); 
    ylabel("AGB Death");
    set(gca,'yscale','log')
    grid on;
%     ylim([10^4,10^6])
    
    nexttile(4);
    xlabel("Central Radius of Ring (kpc)"); 
    ylabel("SNIa Rate");
    set(gca,'yscale','log')
    grid on;
    
%     nexttile(5);
%     xlabel("Central Radius of Ring (kpc)"); 
%     ylabel("NSM Rate");
%     set(gca,'yscale','log')
    
    nexttile(5,[2,2]);
    lw = 2;
    plot(timeset,totalBirth/10^9,'LineWidth',lw);
    hold on;
    plot(timeset,totalCCSN/10^9,'LineWidth',lw);
    plot(timeset,totalAGB/10^9,'LineWidth',lw);
    plot(timeset,totalSNIa/10^9,'LineWidth',lw);
    plot(timeset,totalNSM/10^9,'LineWidth',lw);
    plot(timeset,totalECSN/10^9,'LineWidth',lw);
    hold off;
    xlabel("Simulation Time (Gyr)");
    ylabel("Events per yr");
    legend(["Star Birth","CCSN","AGB Death","SNIa","NSM","ECSN"],"location","southeast");
    title(T,fileName);
    set(gca,'yscale','log');
    set(gca,'xscale','linear');
    grid on;
end