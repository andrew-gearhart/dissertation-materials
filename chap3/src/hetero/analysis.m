% Matlab script to fit model parameters in various ways depending on
% which lines are commented/uncommented
format long g

fitData = zeros(2,9);

% read in the data that has already been column-scaled (see
% lsqTry.xlsx). Format is [F W T E]
dgemmMKL=csvread('sgemmHeteroScaled.csv');
dgemvMKL=csvread('sgemvHeteroScaled.csv');

%dgemmMKL=csvread('dgemmMKLEmerald.csv');
%dgemvMKL=csvread('dgemvMKLEmerald.csv');

% no row scaling
dgemmMKLD = diag(ones(length(dgemmMKL),1));
dgemvMKLD = diag(ones(length(dgemvMKL),1));

% 1/F row scaling...WARNING GPU should be scaled to its own flops, not
% CPU! What about energy??
%dgemmMKLD = diag(1.0./dgemmMKL(:,1));
%dgemvMKLD = diag(1.0./dgemvMKL(:,1));

% 1/F^{2/3},1/F^{1/2},1/F/2
%dgemmMKLD = diag(1.0./dgemmMKL(:,1).^(2.0/3.0));
%dgemvMKLD = diag(1.0./dgemvMKL(:,1).^(1.0/2.0));

dgemmMKLRS = dgemmMKLD*dgemmMKL;
dgemvMKLRS = dgemvMKLD*dgemvMKL;

% the normal nonnegative fits, all implementations seperate
%fitData(1,:) = [lsqnonneg(dgemmMKLRS(:,1:2),dgemmMKLRS(:,6))',lsqnonneg(dgemmMKLRS(:,3:4),dgemmMKLRS(:,7))',lsqnonneg(dgemmMKLRS(:,1:5),dgemmMKLRS(:,8))'];
%fitData(2,:) = [lsqnonneg(dgemvMKLRS(:,1:2),dgemvMKLRS(:,6))',lsqnonneg(dgemvMKLRS(:,3:4),dgemvMKLRS(:,7))',lsqnonneg(dgemvMKLRS(:,1:5),dgemvMKLRS(:,8))'];

% regular least squares, all implementations seperate
%fitData(1,:) = [(dgemmMKLRS(:,1:2)\dgemmMKLRS(:,6))',(dgemmMKLRS(:,3:4)\dgemmMKLRS(:,7))',(dgemmMKLRS(:,1:5)\dgemmMKLRS(:,8))'];
%fitData(2,:) = [(dgemvMKLRS(:,1:2)\dgemvMKLRS(:,6))',(dgemvMKLRS(:,3:4)\dgemvMKLRS(:,7))',(dgemvMKLRS(:,1:5)\dgemvMKLRS(:,8))'];

% one fit to rule them all
AtimeCPU = [dgemmMKLRS(:,1:2);dgemvMKLRS(:,1:2)];
btimeCPU = [dgemmMKLRS(:,6);dgemvMKLRS(:,6)];
AtimeGPU = [dgemmMKLRS(:,3:4);dgemvMKLRS(:,3:4)];
btimeGPU = [dgemmMKLRS(:,7);dgemvMKLRS(:,7)];
Aenergy = [dgemmMKLRS(:,1:5);dgemvMKLRS(:,1:5)];
benergy = [dgemmMKLRS(:,8);dgemvMKLRS(:,8)];

% row scaling by 1/F and 1/E
AtimeDCPU = diag(1.0./AtimeCPU(:,1));
AtimeDGPU = diag(1.0./AtimeGPU(:,1));
AenergyD = diag(1.0./benergy);

% row scaling to 1/T or 1/E
%AtimeDCPU = diag(1.0./btimeCPU);
%AtimeDGPU = diag(1.0./btimeGPU);
%AenergyD = diag(1.0./benergy);

AtimeRSCPU = AtimeDCPU*AtimeCPU;
btimeRSCPU = AtimeDCPU*btimeCPU;
AtimeRSGPU = AtimeDGPU*AtimeGPU;
btimeRSGPU = AtimeDGPU*btimeGPU;
AenergyRS = AenergyD*Aenergy;
benergyRS = AenergyD*benergy;

%fitData(1,:) = [lsqnonneg(AtimeCPU,btimeCPU)',lsqnonneg(AtimeGPU,btimeGPU)',lsqnonneg(Aenergy,benergy)'];
%fitData(2,:) = [lsqnonneg(AtimeCPU,btimeCPU)',lsqnonneg(AtimeGPU,btimeGPU)',lsqnonneg(Aenergy,benergy)'];

fitData(1,:) = [lsqnonneg(AtimeRSCPU,btimeRSCPU)',lsqnonneg(AtimeRSGPU,btimeRSGPU)',lsqnonneg(AenergyRS,benergyRS)'];
fitData(2,:) = [lsqnonneg(AtimeRSCPU,btimeRSCPU)',lsqnonneg(AtimeRSGPU,btimeRSGPU)',lsqnonneg(AenergyRS,benergyRS)'];


%fitData(1,:) = [(AtimeRS\btimeRS)',(AenergyRS\benergyRS)'];
%fitData(2,:) = [(AtimeRS\btimeRS)',(AenergyRS\benergyRS)'];


fitData'
