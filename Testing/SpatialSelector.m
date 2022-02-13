set(0,'defaultTextInterpreter','latex');
set(0,'defaultAxesFontSize',20);
set(groot, 'defaultAxesTickLabelInterpreter','latex'); set(groot, 'defaultLegendInterpreter','latex');
global dSol phi0 nNorm zMin kappa zMax bCut southOverride
phi0 = 1;
dSol = 8.2;
nNorm = 1;
zMin = 0.1;
kappa = 0.1;
bCut = 0;
southOverride = true;
r = 4;


% main = waitbar(0,'Total Progress');
loop = waitbar(0,'Integrating');
% waitboy = waitbar(0,'Creating selection function');

clf

width = 1;
Nrings = 20;

b = 3;
times = [0.1,10];
Ntimes = length(times);
cs = jet(Nrings);
radii = linspace(1,20,Nrings);
N = 100;
Mvs = linspace(-3,8,N);
vals = zeros(size(Mvs));
c= 1;
T = tiledlayout(3,Ntimes);
modenames = ["Magnitude Only", "Avoiding Disc","Southern Skies"];
for mode = 1:3
    
    if mode == 1
        southOverride = true;
        bCut = 0;
    end
    if mode == 2
        southOverride = true;
        bCut = 10;
    end
    if mode == 3
        southOverride = false;
        bCut = 10;
    end
    
    for i = 1:Nrings
    for k = 1:Ntimes
        nNorm = 1;
        nNorm = 1/trapz(zs,n(1,zs,times(k)));
        
        nexttile(k + Ntimes * (mode-1));
        r = radii(i);
        for j = 1:N
            waitbar(j/N,loop,'Integrating');
            vals(j) = Fraction(r,width,Mvs(j),times(k),4);
        end
        time = times(k);
   
        hold on;
        mV = min(vals(vals>0));
        vals(vals == 0) = min(1e-15,mV);
        plot3(Mvs,ones(size(vals)) * r, vals,'Color',cs(i,:),'LineWidth',3);
        zlim([1e-6,1]);
        xlim([-3,8]);
        set(gca,'zscale','log');
        set(gca,'yscale','linear');
        title(modenames(mode) + " for ages $\tau = "+ num2str(times(k)) + "$");
        view([0,0]);
        grid on;
        drawnow;
        hold off;
        
        if (k == Ntimes)
            colormap(jet);
            c = colorbar;
            caxis([min(radii),max(radii)])
            c.Label.String = "Annulus Radius (kpc)";
            c.Label.Interpreter = "Latex";
            c.TickLabelInterpreter = "latex";
        end
%     waitbar(i/Nrings,main,'Creating selection function');
    end
    end
end
xlabel(T,"Absolute Visual Magnitude","Interpreter","latex","FontSize",25);
ylabel(T,"Fraction of Stars Entering Survey","Interpreter","latex","FontSize",25);

delete(loop);
function frac = Fraction(radius,width,Mv,time,resolution)
    global dSol zMax
    
    down = 10^( (2 - Mv)/5);
    up = 10^( (4 - Mv)/5);
    
    biggerThanDisc = (down -dSol >= (radius + width/2));
    smallerThanDisc = (up + radius + width/2< dSol);
    
    if ( smallerThanDisc)
        frac= 0;
        return
    end
    
    
    phi = linspace(0,2*pi,180*resolution);
    
    rs = linspace(radius - width/2, radius + width/2,resolution);
    rVals = zeros(size(rs));
    zRes = 50;
    zs = 10.^(linspace(-3,log10(zMax),zRes));
    zVals = zeros(zRes,1);

    
    
     %%global property check
 
 
%     nNorm = 1;
%     nNorm = 1/trapz(zs,n(radius,zs));
 
    for i = 1:resolution
       r = rs(i);

       rVals(i) = trapz(phi,r*S(r,phi,Mv,time));
    end

    frac = trapz(rs,rVals) / (2 * pi * radius * width);

    
   
end
function v = S(r,phi,Mv,tau)
    
    down = 10^( (2 - Mv)/5);
    up = 10^( (4 - Mv)/5);

    v = zIntegral(r,phi,up,down,tau);
end

function n = n(r,z,tau)

    global kappa zMin nNorm
    
    zBar = zMin + kappa * tau^(2/3);
    n = 1/zBar * exp(-z./zBar);
    
%     n = nNorm * ones(size(z));
end

function fs = zIntegral(r,phis,maxRadius,minRadius,tau)
    global dSol
     global kappa zMin bCut
    fs = zeros(size(phis));
    for i = 1:length(phis)
        phi = phis(i);
        zBar = zMin + kappa * tau^(2/3);
        dPlane = sqrt(dSol^2 + r.^2 - 2 * dSol * r * cos(phi));
        if maxRadius < dPlane
            f= 0;
            return
        end

        aPlus = sqrt(maxRadius^2 - dPlane^2);
        offPlane = dPlane * tand(bCut);
        aMinus = sqrt( max(offPlane^2, minRadius^2 - dPlane^2));

        zC = southCut(r,phi,dPlane);

        zPlus = min(aPlus,zC);
        zMinus = aMinus;

        upIntegral = 0;
        if (zPlus > zMinus)
            upIntegral = 0.5 * (exp(-abs(zMinus)/zBar) - exp(-abs(zPlus)/zBar));
        end

        qPlus = abs(aPlus);
        qMinus = abs(min(-aMinus,zC));
        downIntegral = 0;
        if (qPlus > qMinus)
            downIntegral = 0.5 * (exp(-abs(qMinus)/zBar) - exp(-abs(qPlus)/zBar));
        end

        fs(i) = upIntegral + downIntegral;
    end
end

function z = southCut(r,phi,dPlane)

    ell = asind( r/dPlane * sin(phi));
    C = [-1.241,-0.1148];
    D = [1.916,-0.255];
    
    tanb = 0.716;
    for i = 1:length(C)
       tanb = tanb + C(i) * cosd(i*ell) + D(i) * sind(i*ell);  
    end
    global southOverride
    if southOverride
        tanb = 99999;
    end
    z = dPlane * tanb;
end
