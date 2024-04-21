%bilayered first order filter (default)
filter = filterHairpin;
show(filter);

%charge distribution
figure;
charge(filter,2e9);

%optimizing parameters for L-Band
filter.Resonator = ubendCurved;
filter.FilterOrder = 3;
filter.ResonatorOffset = [0 0 0];
filter.Spacing = [4.000e-04 4.000e-04];
filter.PortLineLength = 0.008;
filter.PortLineWidth = 0.005;
filter.FeedOffset = [-0.0055 -0.0055];
filter.FeedType = "Tapped";
filter.CoupledLineLength = 0.00297;
filter.CoupledLineWidth = 0.0036;
filter.Height = 0.0016;
filter.GroundPlaneWidth = 0.0567;

%impedance
Z0 = getZ0(microstripLine);

%response plot
m = microstripLine(Length=0.08,Width=0.005,Height=0.0016,GroundPlaneWidth=0.0567);
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
