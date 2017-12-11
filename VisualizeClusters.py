
__author__ = 'Chris'

import os
import sys
import csv
import string
import datetime
import statistics
import numpy as np
import glob
import time
import itertools
import matplotlib.pyplot as mplt
#from pylab import *
from collections import defaultdict

def     SpreadCmp( CL1, CL2 ):
    if CL1.GetSpread() < CL2.GetSpread() : return -1
    elif CL1.GetSpread() > CL2.GetSpread(): return 1
    else: return 0

def CalcDist( vec1, vec2 ):
    if not len(vec1) == len(vec2):
        return 99999.
    else:
        sumsq = 0.0
        for i in range(0,len(vec1)):
            sumsq += ((vec1[i]-vec2[i])/vec2[i])**2
        return sqrt(sumsq)

class Stock:
    
    def __init__(self, symb):
        self.Symbol     = symb
    def AddFactors(self, factors ):
        self.Factors = factors
    def AddFactor(self, factor ):
        self.Factors.append(factor)
    def EditFactor(self, factor, pos ):
        if pos < 0 or pos >= len(self.Factors):
            print "Could not edit factor at position %d. Out of range!" % pos
            return 0
        else:
            self.Factors[pos] = factor
    def GetSymbol(self):
        return self.Symbol
    def GetFactors(self):
        return self.Factors
    def GetFactor( self, pos ):
        if pos < 0 or pos > len(self.Factors):
            print "Could not return factor at position %d. Out of range!" % pos
            return 0
        else:
            return self.Factors[pos]


class Cluster:
    def __init__(self, number, cent = []):
        self.Number = number
        self.Size   = 0
        self.Stocks = []
        self.Center = cent
    def AddStock(self, stock):
        self.Stocks.append( stock );
        self.Size += 1
    def GetCenter(self):
        if not self.Size:
            print "There are no stocks in this cluster"
            return 0
        else:
            return self.Center / self.Size
    def GetCenter( self, pos ):
        if pos < 0 or pos > 3:
            print "Could not return center at position %d. Out of range!" % pos
            return 0
        elif not self.Size:
            print "There are no stocks in this cluster"
            return 0
        else:
            return self.Center[pos] / self.Size
    def CalculateSpread( self ):
        sumdist     = 0.0
        sumdist2    = 0.0
        for stk in self.Stocks:
            dist = CalcDist( stk.Factors, self.Center )
            sumdist     += dist
            sumdist2    += dist*dist
        if not self.Size > 1: self.Spread = 0.0
        else:
            self.Spread = sqrt( (sumdist2 - sumdist*sumdist/self.Size) / (self.Size-1) )
    def GetSpread(self):
        return self.Spread
 
class Trial:
    def __init__(self, factors):
        self.Clusters = []
        self.Nclusters = 0
        self.Nfactors  = factors
        if ( self.Nfactors > 5 ): self.Nfactors = 5
    def AddCluster(self, cluster):
        self.Clusters.append(cluster)
        self.Nclusters += 1
    def PlotIteration(self,):
        fig, plot01 = mplt.subplots( self.Nfactors-1,1,sharex=True)
        mins = [ 9999. for i in range(self.Nfactors) ]
        maxs = [ -9999. for i in range(self.Nfactors) ]
        Cindx =0
        for C in self.Clusters:
            Fs = [ [] for i in range(self.Nfactors) ]
            clustcolor = colors.next()
            markerstyle = markers.next()
            C0 = [[C.Center[i]] for i in range(self.Nfactors)]
            Sindx=0
            for S in C.Stocks:
                for i in range(self.Nfactors):
                    Fs[i].append(S.GetFactor(i))
                    if ( Fs[i][Sindx] > maxs[i]): maxs[i] = Fs[i][Sindx]
                    if ( Fs[i][Sindx] < mins[i]): mins[i] = Fs[i][Sindx]
                Sindx += 1
            Label = "C%d" % C.Number
            for i in range(1,self.Nfactors):
                CentLabl = "%d" % C.Number
                plot01[i-1].scatter(Fs[0], Fs[i], color=clustcolor, marker=markerstyle, alpha=0.5, label=Label)
                plot01[i-1].scatter(C0[0], C0[i], color='k', marker='X', alpha=0.5)
                plot01[i-1].set_ylim([mins[i],maxs[i]])
                plot01[i-1].set_xlabel('F0')
                plot01[i-1].set_ylabel( 'F%d' % i )
                plot01[i-1].set_title('F%d/F0' % i)
            Cindx += 1
            if Cindx >= 20: break;
        mplt.legend(bbox_to_anchor=(0.95, 3.5), loc=2, borderaxespad=0.)
        mplt.show()
        


##########################################################################################
ItersList       = []
ClusterList     = []
NClusters       = 0
np.random.seed(11101)
markers = itertools.cycle((',','+','.','o','*'))
colors  = itertools.cycle(('xkcd:blue','xkcd:red','xkcd:violet','xkcd:magenta','xkcd:teal', 'xkcd:lime green', 'xkcd:orange'))
def     TotSpdCmp( CL1, CL2 ):
    if CL1.GetTotSpread() < CL2.GetTotSpread(): return -1
    elif CL1.GetTotSpread() > CL2.GetTotSpread(): return 1
    else: return 0
def     LoadClusters( filename ):
    print "Loading file %s" % filename
    with open( filename, 'r' ) as file:
        print "File opened"
        reader = csv.reader( file, delimiter="\t" )
        Stocks  = []
        number  = -1
        size    = 0
        totspd  = 9999.;
        NumFactors = 0
        for line in reader:
            if not len(line): continue
            if line[0] == 'I':
                NumFactors = int(line[1])
                Itr = Trial(NumFactors);
                ItersList.append(Itr)
            elif line[0] == 'C':
                number  = int(line[1])
                size    = int(line[2])
                centroid = []
                for i in range(NumFactors):
                    centroid.append(float(line[3+i]))
                CL = Cluster(number, centroid)
                ItersList[len(ItersList)-1].AddCluster(CL)
            elif line[0] == 'S':
                symbol  = line[1]
                factors = []
                for i in range(NumFactors):
                    factors.append(float(line[2+i]))
                stk     = Stock(symbol);
                stk.AddFactors( factors )
                ItersList[len(ItersList)-1].Clusters[len(ClusterList)-1].AddStock(stk)

            

    
    return True



if __name__ == '__main__':
    if ( len(sys.argv) < 2 ): sys.exit(1)
    Filename = sys.argv[1]
    print "Loading clusters from file %s" % Filename
   
    LoadClusters( Filename )
    ItersList[len(ItersList)-1].PlotIteration()
    

    