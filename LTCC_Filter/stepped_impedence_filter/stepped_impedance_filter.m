%bilayered first order filter (default)
sub = dielectric("FR4","Teflon"); 
sub.Thickness =[0.003 0.001];
filter = filterStepImpedanceLowPass;
filter.Height = 0.003;
filter.Substrate = sub;
figure;
show(filter);

%charge distribution
figure;
charge(filter,5e9);

%optimizing parameters for L-Band
filter.FilterOrder = 3;
filter.PortLineLength = 0.0034;
filter.PortLineWidth = 0.004;
filter.LowZLineWidth = 0.0096;
filter.HighZLineWidth = 5.000e-04;
filter.LowZLineLength = 0.0026;
filter.Height = 0.0016;
filter.GroundPlaneWidth = 0.012;

%impedance
Z0 = getZ0(microstripLine);

%response plot
m = microstripLine(Length=0.034,Width=0.004,Height=0.0016,GroundPlaneWidth=0.012);
layer2d = bendRightAngle( Length=[m.Length/2 m.Length/2],Width=[m.Width m.Width]);
robj = pcbComponent(layer2d);
robj.BoardThickness = m.Substrate.Thickness;
robj.Layers{2} = m.Substrate;
freq = (1:2:40)*100e6;
Sckt1 = sparameters(robj,freq,Behavioral=true);
Sem1 = sparameters(robj,freq);
rfplot(Sckt1,'abs','o')
hold on
rfplot(Sem1,'abs','s')
