__author__ = 'Chris'

import sys, os, getopt
import datetime
import subprocess
from datetime import date, timedelta
from random import shuffle
from IsTradingDay import IsTradingDayUS, AddDaysToTradingDay
from subprocess import call
from pylab import *

import numpy as np

now = date.today()
StartDate   = date(2017,1,4)
EndDate     = date(2017,1,31)
DateDelta   = EndDate-StartDate

alpha = [0.01]


def Divide( a, b ):
    if not b:
        return 0.
    else:
        return float(a)/float(b)
def Regularize( X ):
    Mu = X.mean(axis=0)
    SD = X.std(axis=0)
    Z = (X-Mu)/SD
    BadInd = np.append( np.nonzero(Z>10)[0],np.nonzero(Z<-10)[0]) 
    outBI = open("C:/temp/BadIndices.log","w")
    outBI.write("Bad indices:\n")
    duplicates = []
    Ndups = 0
    for i in np.nditer(BadInd):
        outBI.write("%d:\nZ: " % i )
        if i in duplicates:
            Ndups += 1
        else:
            duplicates.append(i)
        for z in np.nditer(Z[i]):
            outBI.write( "%.2f\t" % z )
        outBI.write("\nX: ")
        for x in np.nditer(X[i]):
            outBI.write( "%.6f\t" % x )
        outBI.write("\n\n")    
    outBI.close()
    Zprime = np.delete(Z,BadInd.tolist(),0)
    return Z.clip(-3.0,3.0), Mu, SD

def Sigmoid( X, theta ):
    return 1/(1 + np.exp(-X.dot(theta)))

def Cost( Hyp, Y ):
    if Hyp.shape != Y.shape:
        print "Hypothesis and Y shapes are not equal!"
        print "HypShape = %s" % str(Hyp.shape)
        print "Yshape = %s" % str(Y.shape)
        return 0.
    M = Hyp.shape[0]
    
    LogHyp = np.log(Hyp)
    LogOneMinusHyp = np.log(1-Hyp)
    OneMinusY      = 1-Y
    C = Y.dot(LogHyp)+OneMinusY.dot(LogOneMinusHyp)
    return Divide(-C,M)

def CreateDataArray( pythlist ):
    M = len(pythlist)
    if not M: 
        print "Input list has no size"
        return None
    return np.array(pythlist)

def GetCorrection( Hyp, X, Y ):
    if Hyp.shape != Y.shape:
        print "Hypothesis and Y shapes are not equal!"
        print "HypShape = %s" % str(Hyp.shape)
        print "Yshape = %s" % str(Y.shape)
        return 0.
    M = Hyp.shape[0]
    C = Hyp-Y
    return np.sum( X * C[:, np.newaxis], axis=0 )/M

def RunSingleGradDesc( Z, Y, alpha, cutoff=1.0E-6 ):
    J = 9999.
    Diff = 9999.
    Niter = 0
    Js = []
    Nvars = Z.shape[1]
    #np.random.seed(314159)
    Theta = 2*np.random.random(Nvars)-1
    MaxTries = 2E4
    #outfile = open("C:/temp/out.txt","w")
    while ( Diff > cutoff and Niter < MaxTries ):
        Hyps = Sigmoid(Z,Theta)
        DeltaTheta = GetCorrection( Hyps, Z, Y )
        Theta -= alpha * DeltaTheta
        J = Cost(Hyps, Y)
        if Niter: Diff = Js[-1]-J
        if Diff < 0: 
            #print "Final: Niter = %d: J = %.6f, Diff = %.6f" % ( Niter, J, Diff )
            break
        Js.append(J)
        Niter += 1
        if ( Diff <= cutoff or Niter >= MaxTries ):
            pass
            #print "Final: Niter = %d: J = %.6f, Diff = %.6f" % ( Niter, J, Diff )
        #if ( Niter < 10 or Niter % 1000 == 0 ):
        #    print "Niter = %d: J = %.6f, Diff = %.6f" % ( Niter, J, Diff )
    
    #print "\nParameters:"
    #print "%s" % str(Theta)
    return J, Theta

def RunGradientDescent( X, Y, alpha, cutoff=1.0E-6 ): 
    Z, Mus, SDs = Regularize(X)
    print "Mu = %s\nSDs = %s" % ( str(Mus), str(SDs) )
    Z = np.concatenate( (np.ones(Z.shape[0])[:, np.newaxis], Z), axis=1 )
    
    MaxS = [ -9999. for x in range(Z.shape[1]) ]
    PassingIndices = []
    PassingParams = []
    NVARS       = Z.shape[1]
    NENTRIES    = Z.shape[0]
    NEXPTS      = int(NENTRIES / 10000)
    print "Nvars = %d, Nentries = %d, Nexpts = %d" % ( NVARS, NENTRIES, NEXPTS )
    SumThetas   = [ 0. for i in range(NVARS) ]
    SumThetas2  = [ 0. for i in range(NVARS) ]
    Ntries = 0
    for i in range(0,NEXPTS):
        #print "Experiment %d" % i
        Start = i*10000;
        End   = Start + 10000;
        if End >= NENTRIES:
            End = -1
        Zp = Z[Start:End]
        Yp = Y[Start:End]
        J, Theta = RunSingleGradDesc( Zp, Yp, alpha, cutoff )
        indx=0
        for t in np.nditer(Theta):
            if ( np.isnan(t) ):
                print "NaN found for parameter %d in expt %d" % ( indx, i )
            SumThetas[indx] += t
            SumThetas2[indx] += t*t            
            indx += 1
        Ntries += 1
    
    for i in range(0, NVARS):
        AverageTheta = SumThetas[i]/Ntries
        DevTheta = sqrt( (SumThetas2[i] - SumThetas[i]*SumThetas[i]/Ntries)/(Ntries-1) )
        S = abs(AverageTheta)/DevTheta
        #if ( S > 1 ): 
        print "Parameter %d: mu = %.5e, dev = %.5e, S = %.4f" % ( i, AverageTheta, DevTheta, S )
            #PassingIndices.append(i)
            #PassingParams.append(AverageTheta)
                
    if len(PassingIndices) <= 1:
        print "Regression failed, no parameters meet statistical requirements"
    else:
        print "Passing indices = %s\nParameters = %s" % ( str(PassingIndices), str(PassingParams) )


class CTradeData:
    def __init__(self, key ):
        self.Xdata = []
        self.Ydata = []
        self.Nrows = 0
        self.Name = key
    def Insert( self, data ):
        self.Xdata.append(data[:-1])
        self.Ydata.append(data[-1])
        self.Nrows += 1

def GetDates():
    Dates = []
    for d in range( DateDelta.days + 1 ):
        newdate = StartDate + timedelta(days=d)
        if IsTradingDayUS(newdate):
            Dates.append(newdate)

    return Dates


def GetData( version, date, datadict={} ):
    #key to fields:
    #0 keyname, 1 delta, 2 Ratio, 3 Zscore, 4 Zratio, 5 Age, 6 RatSlope, 7 CummRtnSlope, 8 Rtn
    filename = "C:/q2/dev/logs/TradeRegData/TRD_%s_%s.dat" % ( version, date.strftime("%Y%m%d") )
    with open( filename, "r" ) as infile:
        lncount = 0
        for ln in infile:
            fields  = ln.split("\t")
            key     = fields[0]
            TradeDat= CTradeData(key)
            data    = [float(f) for f in fields[1:]]
            if key not in datadict:
                datadict[key] = TradeDat
            datadict[key].Insert( data )
            lncount += 1

    
def WorkData( datadict ):
    X = []
    Y = []
    W = []
    for k in datadict:
        for i in range( 0, datadict[k].Nrows-4 ):
            x = datadict[k].Xdata[i][5:]# + [abs(datadict[k].Xdata[i][0])] + [datadict[k].Ydata[i]]
            #X.append( x )
            #if len(X) >= 4337 and len(X) <= 4338:
            #    print "Row 73885 found, key = %s\t%s" % (k, str(x))
            Rnow = datadict[k].Ydata[i]
            y = 1.
            for j in range( i+1, i+2 ):
                if datadict[k].Ydata[j] > Rnow:
                    y = 0.
                    break
            #Y.append(y)
            W.append(x + [y])
    print "Len(W[0]) = %d, %d" % (len(W[0]), len(W[0][:-1]))
    shuffle(W)
    for w in W:
        X.append( w[:-1] )
        Y.append( w[-1] )
    return X, Y



if __name__ == '__main__':
    if len(sys.argv) != 2:
        print "Usage: %s <Scenario>" % sys.argv[0]
        sys.exit(1)

    Version = sys.argv[1]
    Dates = GetDates()
    MasterDict = {}
    print "Collecting data from files..."
    for d in Dates:
        GetData(Version, d, MasterDict)
    print "Collating data..."
    MasterXlist, MasterYlist = WorkData( MasterDict )
    print "Creating numpy Arrays..."
    X = CreateDataArray(MasterXlist)
    print "\tXdata is %s array" % str(X.shape)
    Y = CreateDataArray(MasterYlist)
    print "\tYdata is %s array" % str(Y.shape)
    print "Running gradient descent..."
    minJ = 9999
    MinParams = np.array(len(MasterXlist[0])+1)
    for a in alpha:
        print "Alpha = %.2f" % a
        RunGradientDescent(X,Y,a)
                

