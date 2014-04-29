#! /usr/bin/python

import time
import numpy
import veusz.embed as veusz
import os
import glob

curdir = os.getcwd();

#DEFINE TEMPLATE TASK HERE!
task = "level_buffer_unsmooth"

print "Curring working dir = " + curdir

files = glob.glob(curdir + "/../output/*.txt" );

for f in  files:
	print "Processing File: " + f

	g = veusz.Embedded(f)
	g.EnableToolbar()

	g.AddImportPath(curdir)
	g.ImportFileCSV(f, linked=True, headermode='1st')

	#copy veuzs commnads here...
	g.Add('page', name='page1', autoadd=False)
	g.To('page1')
	g.Set('width', u'15cm')
	g.Add('graph', name='graph1', autoadd=False)
	g.To('graph1')
	g.Set('leftMargin', '1.37cm')
	g.Set('rightMargin', '0.279cm')
	g.Set('topMargin', '0.315cm')
	g.Set('bottomMargin', '3.84cm')
	g.Add('axis', name='x', autoadd=False)
	g.To('x')
	g.Set('label', u'Segments')
	g.To('..')
	g.Add('axis', name='y', autoadd=False)
	g.To('y')
	g.Set('label', u'Level/Unsmooth/Buffered')
	g.Set('direction', 'vertical')
	g.To('..')
	g.Add('xy', name='xy1', autoadd=False)
	g.To('xy1')
	g.Set('xData', u'SegmentNr')
	g.Set('yData', u'Level')
	g.Set('marker', u'diamond')
	g.Set('markerSize', u'1.8pt')
	g.Set('thinfactor', 3)
	g.Set('key', u'Level')
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
	g.Set('key', u'Unsmooth Second(s)')
	g.Set('PlotLine/color', u'red')
	g.Set('PlotLine/width', u'1pt')
	g.Set('MarkerLine/color', u'red')
	g.Set('MarkerLine/width', u'1pt')
	g.Set('MarkerFill/color', u'red')
	g.To('..')
	g.Add('xy', name='xy3', autoadd=False)
	g.To('xy3')
	g.Set('xData', u'SegmentNr')
	g.Set('yData', u'Buffer')
	g.Set('marker', u'star')
	g.Set('markerSize', u'1.8')
	g.Set('thinfactor', 3)
	g.Set('key', u'Bufferd Second(s)')
	g.Set('PlotLine/color', u'green')
	g.Set('PlotLine/width', u'1pt')
	g.Set('MarkerLine/color', u'green')
	g.Set('MarkerLine/width', u'1pt')
	g.To('..')
	g.Add('key', name='key1', autoadd=False)
	g.To('key1')
	g.Set('horzPosn', 'manual')
	g.Set('vertPosn', 'manual')
	g.Set('horzManual', 0.21861034659230705)
	g.Set('vertManual', -0.33582253248316685)
	g.To('..')
	g.To('..')
	g.To('..')

	#print pdf file 
	g.Export(f[:-4] + "-" +  task + ".pdf")

	#Waiting
	time.sleep(0.5)	

	g.Close();

print "Done!"
