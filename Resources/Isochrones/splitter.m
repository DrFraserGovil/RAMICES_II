Y = @(z) 0.2485+ 1.78 * z;
X = @(m,z) z./(10.^(m + log10(0.0207)));
Z = @(m) (1 - 0.2485)./(2.78 + 1/0.0207 * 10.^(-m));

m = -2.2:0.01:0.5;

clf;
plot(m,Z(m));
% hold on;
% plot(m,X(m,Z(m)));
% plot(m,Y(Z(m)));
% hold off;
set(gca,'yscale','log');
grid on