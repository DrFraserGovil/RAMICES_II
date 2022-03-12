dir = "../Output/Calibration/Yields/";
file = dir + "CCSN_yields.dat";

colormap(parula)
plotCCSNGrid(file,["Mg","O"]);



function plotCCSNGrid(file,elements)
    ccsn = readtable(file);
    xC = ccsn.Mass;
    xC(xC == max(xC)) = 100;
    cut = xC > 36;
    xC = xC(cut);
    yC = 10.^ccsn.logZ(cut);

    uX = unique(xC);
    uY = unique(yC);
    [X,Y] = meshgrid(uX,uY);
    cla;
    T = tiledlayout(length(elements),1,"TileSpacing","None");
    for element = elements
        
        zC = ccsn.(element)(cut);
        
        Z = X;
        xx = 1;
        yy = 1;
        for i = 1:height(zC)
           Z(yy,xx) = zC(i);
%            xx = xx + 1;
           yy = yy + 1;
%            if (xx > length(uX))
%                xx = 1;
%            end
           if (yy > length(uY))
               yy = 1;
               xx = xx + 1;
           end
        end
        
        nexttile;

        surf(X,Y,Z,'LineStyle','None');
        set(gca,'yscale','log');
        shading interp
        view(2);
        xlim([37,100]);
        c = colorbar;
        c.Label.String= "Fractional Net " + element + " Yield";
        c.Label.FontSize = 22;
        c.Label.Interpreter = "latex";
        c.TickLabelInterpreter = "latex";
        if element ~= elements(end)
           set(gca,'xtick',[1])
           set(gca,'xticklabel',[]) 
%            xtick = [];

        end
    end
    fs = 20;
    xlabel(T,"Stellar Mass ($M_\odot$)","FontSize",fs,"Interpreter","latex");
    
    ylabel(T,"Metallicity ($Z$)","FontSize",fs,"Interpreter","latex");
end