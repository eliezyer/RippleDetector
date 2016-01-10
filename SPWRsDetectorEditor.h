/*
    ------------------------------------------------------------------

    This file is part of the Open Ephys GUI
    Copyright (C) 2013 Open Ephys

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


#ifndef __SPWRSDETECTOREDITOR_H_136829C6__
#define __SPWRSDETECTOREDITOR_H_136829C6__

#include "../../../JuceLibraryCode/JuceHeader.h"

#include "../Editors/GenericEditor.h"
#include "../Editors/ElectrodeButtons.h"

class SPWRsInterface;
class SPWRsDetector;
class ElectrodeButton;

/**
  User interface for the Sharp Wave Ripples Detector processor.
*/

class SPWRsDetectorEditor : public GenericEditor,
    public ComboBox::Listener
{
public:
    SPWRsDetectorEditor(GenericProcessor* parentNode, bool useDefaultParameterEditors);

    virtual ~SPWRsDetectorEditor();

    void buttonEvent(Button* button);

    void comboBoxChanged(ComboBox* c);

    void updateSettings();

    void saveCustomParameters(XmlElement* xml);
    void loadCustomParameters(XmlElement* xml);

private:

    ScopedPointer<ComboBox> detectorSelector;

    ScopedPointer<UtilityButton> plusButton;

    void addDetector();

    // ScopedPointer<ComboBox> inputChannelSelectionBox;
    // ScopedPointer<ComboBox> outputChannelSelectionBox;

    // ScopedPointer<Label> intputChannelLabel;
    // ScopedPointer<Label> outputChannelLabel;

    OwnedArray<SPWRsInterface> interfaces;

    int previousChannelCount;

    Array<Colour> backgroundColours;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(SPWRsDetectorEditor);

};

class SPWRsInterface : public Component,
    public ComboBox::Listener,
    public Button::Listener
{
public:
    SPWRsInterface(SPWRsDetector*, Colour, int);
    ~SPWRsInterface();

    void paint(Graphics& g);

    void comboBoxChanged(ComboBox*);
    void buttonClicked(Button*);

    void updateChannels(int);

    void setPhase(int);
    void setInputChan(int);
    void setOutputChan(int);
    void setGateChan(int);

    int getPhase();
    int getInputChan();
    int getOutputChan();
    int getGateChan();

private:

    Colour backgroundColour;

    Path sineWave;
    Font font;

    int idNum;

    SPWRsDetector* processor;

    OwnedArray<ElectrodeButton> phaseButtons;

    ScopedPointer<ComboBox> inputSelector;
    ScopedPointer<ComboBox> gateSelector;
    ScopedPointer<ComboBox> outputSelector;

};

#endif  // __SPWRSDETECTOREDITOR_H_136829C6__