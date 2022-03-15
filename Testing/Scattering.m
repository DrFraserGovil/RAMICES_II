files = "../Output/" + ["Movement300","Pollutant/Active/"] + "/Mass.dat";
% files = "../Output/Pollutant/Active/Mass.dat";
clf
for i = 1:length(files)
    figure(i);
    
%     clf;
    estimateScattering(files(i),i);
    
end



function estimateScattering(fileName,qq)

    f = readtable(fileName);
    styles = ["-","--",":","-."];
%     style = styles(q);
    time = unique(f.Time);
    radius = unique(f.Radius);
    Nr = length(radius)
     dt = time(2) - time(1);
    Ks = {};
    ms = {};
    for i = 1:length(time)
       t = time(i);
       
       masses = f.TotalMass(f.Time == t);
      
       ms{i} = masses;
       sf{i} = f.SurfaceArea(f.Time == t);
        Ks{i} = computeK(ms{i},radius,dt);
        
%         hold on;
%         plot(radius,ms{i}./sf{i})
%         hold off;
        
    end
%     f(1:10,:)


%     hold on;
%     scatter(radius,ms{1}./sf{1})
%     hold off;
%     return

    rs = [5,8,11,16] - 0.1;
%     rs= 8-0.1;
%     rs = 1.9;
    rIDs = getClosestIDs(radius,rs);
 
    
    birthTimes = [0,4];
    
    ts = [3:0.25:10];
    
%     if (qq == 1)
        T = tiledlayout(length(birthTimes),1);
        xlabel(T,"Radius (kpc)","Interpreter","latex","FontSize",20);
        ylabel(T,"Probability Surface Density (kpc$^{-2}$)","Interpreter","latex","FontSize",20);
%     end
    
    norms = zeros(size(rIDs));
    for i = 1:length(birthTimes)
        
         norms = zeros(size(rIDs));
        nexttile(i);
         minT = ts(1);
         maxT = 0;
       start = birthTimes(i); 
       
       c = jet(sum(start + ts < time(end)));
       if qq == 2
           c = parula(sum(start + ts < time(end)));
       end
        startIdx = find(time == start);
       for j = 1:length(ts)
          
           endTime = start+ts(j);
           
           if endTime <= time(end)
               maxT = ts(j);
               timeSteps = ts(j)/dt;

               K_compound = eye(length(radius));



               for k = 1:timeSteps
                  K_compound = Ks{startIdx + k} * K_compound; 
               end
               
%                 K_compound(1:10,1:10)
                
               for q = 1:length(rIDs)
                   spike = zeros(Nr,1);
                    spike(rIDs(q)) = 1;
               
                    output = K_compound * spike;
%                     output(output < 1e-7) = NaN;
                   hold on;
                    output = output./sf{1};
                   if norms(q) == 0
                       norms(q) = max(output);
%                        norms
                   end
%                    [j,size(c),minT,maxT]
                   plot(radius,output,styles(q),'Color',c(j,:),'LineWidth',3);
%                    scatter(radius,output,5,c(j,:));
                    
                    hold off;
%                 end
%                   set(gca,'yscale','log');
               
                    drawnow;
               end
           end
       end
       title("Stars born at " + num2str(start) + "Gyr");
       colormap(jet)
       cb = colorbar;
        cb.Label.String = "Stellar Age (Gyr)";
        
cb.Label.Interpreter = "latex";
        cb.TickLabelInterpreter = "latex";
        [minT,maxT]
        caxis([minT,maxT])
        grid on;
%         ylim([0,0.45])
    end    
%     set(gca,'yscale','log');

end

function K = computeK(ms,rs,dt)
    kappa = 0.5;
    Mt = max(ms);
    dr = rs(2) - rs(1);
    Nr = length(rs);
    
    Nr = length(ms);
    K = zeros(Nr);

    maxProb = kappa * dt / dr^2;
    kappa = maxProb / (dt*Mt);
    for i = 1:Nr
       
        pUp = 0; 
         if i < Nr
             pUp = ms(i+1) * kappa;
             K(i+1,i) = pUp;
         end
         
        pDown = 0;
        if i > 1
            pDown = ms(i-1) * kappa;
            K(i-1,i) = pDown;
        end
        K(i,i) = - pUp - pDown;
        
%         K(1:i+2,1:i+2)
    end
    
    nApply = 1;
%     K = (eye(size(K)) + dt/nApply * K)^(nApply);
    K = expm(dt * K);
%     maxProb
%    K(1:10,1:10)
end

function ids = getClosestIDs(vec,targets)
    ids = zeros(size(targets));
    
    for i = 1:length(targets)
       [~,v] = min(abs(vec - targets(i)));
       ids(i) = v;
    end


end