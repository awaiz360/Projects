%bilayered first order filter (default)
filter = filterInterdigital;
show(filter);

%charge distribution
figure;
charge(filter,2e9);

%optimizing parameters for L-Band
filter.FilterOrder = 5;
filter.ResonatorLength = 0.0204;
filter.ResonatorWidth = 0.0024;
filter.ResonatorOffset = 0;
filter.ResonatorSpacing = [1.3000e-04 3.6000e-04 3.6000e-04 1.3000e-04];
filter.PortLineLength = 0.0049;
filter.PortLineWidth = 0.0019;
filter.FeedOffset = -0.0025;
filter.ViaDiameter = 0.0011;
filter.Height = 0.0013;
filter.GroundPlaneWidth = 0.03;

%impedance
Z0 = getZ0(microstripLine);

%response plot
m = microstripLine(Length=0.0204,Width=0.0024,Height=0.0013,GroundPlaneWidth=0.03);
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
