#! /usr/bin/python

import time
import numpy
import veusz.embed as veusz
import os
import glob

curdir = os.getcwd();

#DEFINE TEMPLATE TASK HERE!
task = "template-task"

print "Curring working dir = " + curdir

files = glob.glob(curdir + "/../output/*.txt" );

for f in  files:
	print "Processing File: " + f

	g = veusz.Embedded(f)
	g.EnableToolbar()

	g.AddImportPath(curdir)
	g.ImportFileCSV('../output/ContentDst.txt', linked=True, headermode='1st')

	#copy veuzs commnads here...
	g.Add('page', name='page1', autoadd=False)
	g.To('page1')
	g.Add('graph', name='graph1', autoadd=False)
	g.To('graph1')
	g.Add('axis', name='x', autoadd=False)
	g.Add('axis', name='y', autoadd=False)
	g.To('y')
	g.Set('direction', 'vertical')
	g.To('..')
	g.Add('xy', name='xy1', autoadd=False)
	g.To('xy1')
	g.Set('xData', u'SegmentNr')
	g.Set('yData', u'Level')
	g.Set('marker', u'diamond')
	g.Set('markerSize', u'1.8pt')
	g.Set('thinfactor', 3)
	g.Set('PlotLine/width', u'1pt')
	g.Set('MarkerLine/width', u'1pt')
	g.To('..')
	g.Add('xy', name='xy2', autoadd=False)
	g.To('xy2')
	g.Set('xData', u'SegmentNr')
	g.Set('yData', u'Unsmooth Second(s)')
	g.Set('marker', u'star')
	g.Set('markerSize', u'1.8')
	g.Set('thinfactor', 3)
	g.Set('PlotLine/color', u'red')
	g.Set('PlotLine/width', u'1pt')
	g.Set('MarkerLine/color', u'red')
	g.Set('MarkerLine/width', u'1pt')
	g.Set('MarkerFill/color', u'red')
	g.To('..')
	g.To('..')
	g.To('..')

	#print pdf file 
	g.Export(f[:-4] + "-" +  task + ".pdf")

	# let the user see the final result
	#print "Waiting for 3 seconds"
	#time.sleep(3)	

	g.Close();

print "Done!"
