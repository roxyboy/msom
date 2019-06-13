#!/usr/bin/env python

import numpy as np
import matplotlib.pyplot as plt
import glob,os,re
import scipy.io.netcdf as netcdf


plt.ion()

dir0 = "../outdir_0005/"
exec(open(dir0 + "params.in").read())

filep = 'po*'
fileq = 'qo*'
filef = 'pf*'

filebf = 'de_bf*'
filevd = 'de_vd*'
filej1 = 'de_j1*'
filej2 = 'de_j2*'
filej3 = 'de_j3*'
fileft = 'de_ft*'

allfilesp = sorted(glob.glob(dir0 + filep));
allfilesq = sorted(glob.glob(dir0 + fileq));
allfilesf = sorted(glob.glob(dir0 + filef));

allfilesbf = sorted(glob.glob(dir0 + filebf));
allfilesvd = sorted(glob.glob(dir0 + filevd));
allfilesj1 = sorted(glob.glob(dir0 + filej1));
allfilesj2 = sorted(glob.glob(dir0 + filej2));
allfilesj3 = sorted(glob.glob(dir0 + filej3));
allfilesft = sorted(glob.glob(dir0 + fileft));

nb_files  = len(allfilesp);

b = np.fromfile(allfilesp[0],'f4')
N = int(b[0])
N1 = N + 1
nl = int(len(b)/N1**2)

fileh = 'dh_' + str(nl) +'l.bin'

dh = np.fromfile(dir0 + fileh,'f4')
dhi = 0.5*(dh[:-1] + dh[1:])

fileFr = 'frpg_' + str(nl) +'l_N' + str(N) + '.bas'
Fr = np.fromfile(dir0 + fileFr,'f4').reshape(nl,N1,N1).transpose(0,2,1)
Fr = Fr[:,1:,1:]

fileppg = 'psipg_' + str(nl) +'l_N' + str(N) + '.bas'
ppg = np.fromfile(dir0 + fileppg,'f4').reshape(nl,N1,N1).transpose(0,2,1)
ppg = ppg[:,1:,1:]

# create grid
Delta = L0/N

x = np.linspace(0.5*Delta, L0 - 0.5*Delta,N)
xc,yc = np.meshgrid(x,x)

# physical parameters
Ro = Rom/(1 + Rom*beta*(yc-0.5*L0));

# backgroud state
upg = -np.gradient(ppg, axis = 1)/Delta*Ro.reshape(1,N,N)
vpg = np.gradient(ppg, axis = 2)/Delta*Ro.reshape(1,N,N)

kepg = 0.5*(upg**2 + vpg**2)

bpg = np.diff(ppg,1,0)/dhi.reshape(nl-1,1,1)
pepg = 0.5*bpg**2*Fr[:-1,:,:]**2

kepg_s = (kepg*dh.reshape(nl,1,1)).sum()*Delta*Delta
pepg_s = (pepg*dhi.reshape(nl-1,1,1)).sum()*Delta*Delta
  
imax = nb_files
#imax = 5

ke_all = np.zeros((imax))
pe_all = np.zeros((imax))

for ifi in range(0,imax):

  p  = np.fromfile(allfilesp[ifi],'f4').reshape(nl,N1,N1).transpose(0,2,1)
  q  = np.fromfile(allfilesq[ifi],'f4').reshape(nl,N1,N1).transpose(0,2,1)
  pf = np.fromfile(allfilesf[ifi],'f4').reshape(nl,N1,N1).transpose(0,2,1)

  p  = p [:,1:,1:]
  q  = q [:,1:,1:]
  pf = pf[:,1:,1:]

  # if len(allfilesbf) > 0:
  #   ebf = np.fromfile(allfilesbf[ifi],'f4').reshape(nl,N1,N1).transpose(0,2,1)
  #   evd = np.fromfile(allfilesvd[ifi],'f4').reshape(nl,N1,N1).transpose(0,2,1)
  #   ej1 = np.fromfile(allfilesj1[ifi],'f4').reshape(nl,N1,N1).transpose(0,2,1)
  #   ej2 = np.fromfile(allfilesj2[ifi],'f4').reshape(nl,N1,N1).transpose(0,2,1)
  #   ej3 = np.fromfile(allfilesj3[ifi],'f4').reshape(nl,N1,N1).transpose(0,2,1)
  #   eft = np.fromfile(allfilesft[ifi],'f4').reshape(nl,N1,N1).transpose(0,2,1)

  #   ebf = ebf[:,1:,1:]
  #   evd = evd[:,1:,1:]
  #   ej1 = ej1[:,1:,1:]
  #   ej2 = ej2[:,1:,1:]
  #   ej3 = ej3[:,1:,1:]
  #   eft = eft[:,1:,1:]  

  pt = p

  u = -np.gradient(pt, axis = 1)/Delta*Ro.reshape(1,N,N)
  v = np.gradient(pt, axis = 2)/Delta*Ro.reshape(1,N,N)
    
  ke = 0.5*(u**2 + v**2)
  
#  pt = p + ppg


  b = np.diff(pt,1,0)/dhi.reshape(nl-1,1,1)
  pe = 0.5*b**2*Fr[:-1,:,:]**2
  
  ke_all[ifi] = (ke*dh.reshape(nl,1,1)).sum()*Delta*Delta
  pe_all[ifi] = (pe*dhi.reshape(nl-1,1,1)).sum()*Delta*Delta
  
tt = np.linspace(0,imax-1,imax)*dtout

pe_all = pe_all - pe_all[0]

plt.figure()
plt.plot(tt,ke_all, label= 'KE')
plt.plot(tt,pe_all, label= 'PE')

plt.ticklabel_format(style='sci', axis='y', scilimits=(0,0))

plt.xlabel("time")
plt.ylabel("E")
plt.legend()

print("Background PE = {0:.1e}".format(pepg_s))
print("Background KE = {0:.1e}".format(kepg_s))