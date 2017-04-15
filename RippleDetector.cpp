/*
    ------------------------------------------------------------------
    This file is part of the Open Ephys GUI
    Copyright (C) 2015 Open Ephys
    ------------------------------------------------------------------
    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.
    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.
    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#include <stdio.h>
#include <math.h>
#include <time.h>
#include <algorithm>
#include "RippleDetector.h"
#include "RippleDetectorEditor.h"

RippleDetector::RippleDetector()
    : GenericProcessor("Ripple Detector"), activeModule(-1),
      risingPos(false), risingNeg(false), fallingPos(false), fallingNeg(false)

{

}

RippleDetector::~RippleDetector()
{

}

AudioProcessorEditor* RippleDetector::createEditor()
{
    return editor = new RippleDetectorEditor(this, true);

    std::cout << "Creating editor." << std::endl;

    //return editor;
}

void RippleDetector::addModule()
{
    DetectorModule m = DetectorModule();
    m.inputChan = -1;
    m.outputChan = -1;
    m.gateChan = -1;
    m.isActive = true;
    m.lastSample = 0.0;
    m.type = NONE;
    m.samplesSinceTrigger = 5000;
    m.wasTriggered = false;
    m.phase = NO_PHASE;
    m.MED = 0.00;
    m.STD = 0.00;
    m.AvgCount = 0;
    m.flag = 0;
    m.tReft = 0.0;
    m.count = 0;
    
    
    modules.add(m);

}


void RippleDetector::setActiveModule(int i)
{
    activeModule = i;

}


void RippleDetector::setParameter(int parameterIndex, float newValue)
{

    DetectorModule& module = modules.getReference(activeModule);

    if (parameterIndex == 1) // module type
    {

        int val = (int) newValue;

        switch (val)
        {
            case 0:
                module.type = NONE;
                break;
            case 1:
                module.type = PEAK;
                break;
            case 2:
                module.type = FALLING_ZERO;
                break;
            case 3:
                module.type = TROUGH;
                break;
            case 4:
                module.type = RISING_ZERO;
                break;
            default:
                module.type = NONE;
        }
    }
    else if (parameterIndex == 2)   // inputChan
    {
        module.inputChan = (int) newValue;
    }
    else if (parameterIndex == 3)   // outputChan
    {
        module.outputChan = (int) newValue;
    }
    else if (parameterIndex == 4)   // gateChan
    {
        module.gateChan = (int) newValue;
        if (module.gateChan < 0)
        {
            module.isActive = true; 
        }
        else
        {
            module.isActive = false;
        }
    }

}

void RippleDetector::updateSettings()
{

}

bool RippleDetector::enable()
{
    return true;
}

void RippleDetector::handleEvent(int eventType, MidiMessage& event, int sampleNum)
{
    // MOVED GATING TO PULSE PAL OUTPUT!
    // now use to randomize phase for next trial

//    std::cout << "GOT EVENT." << std::endl;

    if (eventType == TTL)
    {
        const uint8* dataptr = event.getRawData();

        // int eventNodeId = *(dataptr+1);
        int eventId = *(dataptr+2);
        int eventChannel = *(dataptr+3);

        for (int i = 0; i < modules.size(); i++)
        {
            DetectorModule& module = modules.getReference(i);

            if (module.gateChan == eventChannel)
            {
                if (eventId)
                    module.isActive = true;
                else
                    module.isActive = false;
            }
        }

    }

}

void RippleDetector::process(AudioSampleBuffer& buffer,
                            MidiBuffer& events) //This is the core of the code, is the script that will run when every buffer comes
{
    Time time; //I'm using a library to count time
    checkForEvents(events);
    // loop through the modules
    for (int i = 0; i < modules.size(); i++)
    {
      DetectorModule& module = modules.getReference(i);

        double t;    
        double t3;
        double RefratTime;
            
            
            t = double(time.getHighResolutionTicks()) / double(time.getHighResolutionTicksPerSecond());//Starting to count time for the script here
            double arrSized = round(getNumSamples(module.inputChan)/4);//the following 3 lines are to create an array of specific size for saving the RMS from buffer
            int arrSize = (int) arrSized;
            float RMS[arrSize];
            for (int index = 0; index < arrSize; index++) //here the RMS is calculated
            {
                RMS[index] = sqrt( (
                   pow(buffer.getSample(module.inputChan,(index*4)),2) +
                   pow(buffer.getSample(module.inputChan,(index*4)+1),2) +
                   pow(buffer.getSample(module.inputChan,(index*4)+2),2) +
                   pow(buffer.getSample(module.inputChan,(index*4)+3),2)
                )/4 );
            }
            
            for (int pac = 0; pac < arrSize; pac++)
            {
                if (module.AvgCount < 60000/4) //Using the RMS value in the first 2 s as baseline to build a threshold of detection
                {
                    module.AvgCount++; // all the values that must be saved from one buffer to other has to be save in another function outside the process (this function), when I need to save values to reuse in the next buffer I use the structure module from addModule function
                    float var = RMS[pac];
                    float delta = var - module.MED;
                    module.MED = module.MED + (delta/module.AvgCount); //calculates average for threshold
                    module.STD = module.STD + delta*(var-module.MED); // calculates standard deviation for threhsold
                }
                else
                {
                    break;
                }
            }

            for (int i = 0; i < arrSize; i++)
            {
            
                const float sample = RMS[i];
                double threshold = module.MED + 2.00*sqrt(module.STD/(module.AvgCount*4)); //building the threshold from average + n*standard deviation
                
                if ( sample >=  threshold & RefratTime > 2 ) //counting how many points are above the threshold and if has been 2 s after the last event
                {
                  module.count++;
                }
		else if(sample < threshold & i == 0)//protect from acumulation
		{
		  module.count = 0;
 		}               
                if (module.flag == 1)
                {
                    t3 = ( double(time.getHighResolutionTicks()) / double(time.getHighResolutionTicksPerSecond()) )- module.tReft;//calculating refractory time
                    RefratTime = t3;
                }
                else
                {
                    RefratTime = 3;
                }
       
                if (module.count >= round(0.020*30000/4) & RefratTime > 2 ) //this is the time threshold, buffer RMS amplitude must be higher than threshold for a certain period of time, the second term is the Refractory period for the detection, so it hasn't a burst of activation after reaching both thresholds
                {
                        module.flag = 1;
			module.count = 0;
                        module.tReft = double(time.getHighResolutionTicks()) / double(time.getHighResolutionTicksPerSecond());
                        //double timeStamp2 = (module.tReft - t); 
                        
                        //saving the script delay
                 //       FILE *f = fopen("timeStamps.txt", "a+");
                        
                 //       fprintf(f,"%f\n ",timeStamp2*1000);

                 //       fclose(f); 
                        
    
                        addEvent(events, TTL, i, 1, module.outputChan);
                        module.samplesSinceTrigger = 0;

                        module.wasTriggered = true;
                        //std::cout << averageTime << std::endl;
                }
                module.lastSample = sample;

                if (module.wasTriggered)
                {
                    if (module.samplesSinceTrigger > 1000)
                    {
                        addEvent(events, TTL, i, 0, module.outputChan);
                        module.wasTriggered = false;
                    }
                    else
                    {
                        module.samplesSinceTrigger++;
                    }
                }


        }

    }

}

void RippleDetector::estimateFrequency()
{}

    // int N = (numPeakIntervals < NUM_INTERVALS) ? numPeakIntervals
    //         : NUM_INTERVALS;

    // int sum = 0;

    // for (int i = 0; i < N; i++)
    // {
    //     sum += peakIntervals[i];
    // }

    // estimatedFrequency = getSampleRate()/(float(sum)/float(N));
