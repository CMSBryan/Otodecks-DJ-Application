//Chai Ming Shi Bryan
#include "MusicLibrary.h"
#include "DeckGUI.h"  // Include the full header here
#include "../JuceLibraryCode/JuceHeader.h"
using namespace juce;

MusicLibrary::MusicLibrary()
    : deckPtr1(nullptr),
      deckPtr2(nullptr),
      loadToDeck1Button("Deck 1"),
      loadToDeck2Button("Deck 2"),
      selectedRow(-1)
{
    // Set up table component
    addAndMakeVisible(tableComponent);
    tableComponent.setModel(this);
    tableComponent.getHeader().addColumn("Track", 1, 200);
    tableComponent.getHeader().addColumn("Duration", 2, 100);
    
    // Set up load button
    addAndMakeVisible(loadButton);
    loadButton.setButtonText("Load Files");
    loadButton.onClick = [this] {
        fileChooser = std::make_unique<juce::FileChooser>("Select audio files...",
            juce::File::getSpecialLocation(juce::File::userMusicDirectory),
            "*.wav;*.mp3;*.aif;*.flac");

        fileChooser->launchAsync(juce::FileBrowserComponent::openMode
                          | juce::FileBrowserComponent::canSelectFiles
                          | juce::FileBrowserComponent::canSelectMultipleItems,
            [this](const juce::FileChooser& fc)
            {
                auto results = fc.getResults(); // Array<juce::File>

                for (const auto& file : results)
                {
                    addTrack(file.getFullPathName());
                }
            });
    };

    addAndMakeVisible(removeButton);
    removeButton.setButtonText("Remove Selected");
    removeButton.setColour(juce::TextButton::buttonColourId, juce::Colour(0xffF44336)); // Red color
    removeButton.onClick = [this] {
        int selectedRow = tableComponent.getSelectedRow();
        if (selectedRow >= 0 && selectedRow < displayedTracks.size())
        {
            auto targetTitle = displayedTracks[selectedRow].title;
            tracks.erase(std::remove_if(tracks.begin(), tracks.end(), [&](const Track& t) {
                return t.title == targetTitle;
                }), tracks.end());
            tableComponent.updateContent();             // Refresh the UI
            tableComponent.deselectAllRows();
            saveLibrary();                              // Overwrite the XML to persist deletion
        }
     };

    // Set up search box
    addAndMakeVisible(searchBox);
    searchBox.setTextToShowWhenEmpty("Search tracks...", juce::Colours::grey);
    searchBox.onTextChange = [this] {
        searchTracks(searchBox.getText());
        };



    // Set up title label
    addAndMakeVisible(titleLabel);
    titleLabel.setText("Music Library", juce::dontSendNotification);
    titleLabel.setFont(juce::Font(18.0f, juce::Font::bold));
    titleLabel.setJustificationType(juce::Justification::centred);

    // Set up deck loading buttons
    addAndMakeVisible(loadToDeck1Button);
    loadToDeck1Button.setColour(juce::TextButton::buttonColourId, juce::Colour(0xff4CAF50));
    loadToDeck1Button.onClick = [this]() {
        auto selectedRows = tableComponent.getSelectedRows();
        if (selectedRows.size() > 0)
            loadTrackToDeck(selectedRows[0], 1);
    };

    addAndMakeVisible(loadToDeck2Button);
    loadToDeck2Button.setColour(juce::TextButton::buttonColourId, juce::Colour(0xff2196F3));
    loadToDeck2Button.onClick = [this]() {
        auto selectedRows = tableComponent.getSelectedRows();
        if (selectedRows.size() > 0)
            loadTrackToDeck(selectedRows[0], 2);
    };

    // Load existing library on startup
    loadLibrary();
}

MusicLibrary::~MusicLibrary()
{
    // Save library when component is destroyed
    saveLibrary();
}

void MusicLibrary::paint(juce::Graphics& g)
{
    // Custom paint function for styling
    g.fillAll(juce::Colour(0xff2d2d30)); // Dark background
    g.setColour(juce::Colours::lightgrey);
    g.drawRect(getLocalBounds(), 2);
}

void MusicLibrary::resized()
{
    auto area = getLocalBounds();

    if (area.isEmpty())
        return;

    // Title at top
    if (area.getHeight() > 30)
        titleLabel.setBounds(area.removeFromTop(30));

    // Load and Remove buttons side-by-side below title
    if (area.getHeight() > 40)
    {
        auto topButtons = area.removeFromTop(40);
        auto leftButtonArea = topButtons.removeFromLeft(topButtons.getWidth() / 2);

        loadButton.setBounds(leftButtonArea.reduced(5));
        removeButton.setBounds(topButtons.reduced(5));
    }

    //Search Box
    if (area.getHeight() > 35)
    {
        searchBox.setBounds(area.removeFromTop(35).reduced(5));
    }

    // Deck loading buttons
    if (area.getHeight() > 40)
    {
        auto deckButtonArea = area.removeFromTop(40);
        int halfWidth = deckButtonArea.getWidth() / 2;
        loadToDeck1Button.setBounds(deckButtonArea.removeFromLeft(halfWidth).reduced(5));
        loadToDeck2Button.setBounds(deckButtonArea.reduced(5));
    }

    // Table takes the remaining space
    if (area.getWidth() > 10 && area.getHeight() > 10)
    {
        tableComponent.setBounds(area.reduced(5));
    }
    else
    {
        tableComponent.setBounds(area);
    }
}

int MusicLibrary::getNumRows()
{
    return static_cast<int>(displayedTracks.size());
}

void MusicLibrary::paintRowBackground(juce::Graphics& g, int rowNumber, int width, int height, bool rowIsSelected)
{
    if (rowIsSelected)
        g.fillAll(juce::Colour(0xff42a2c8)); // Blue selection
    else if (rowNumber % 2)
        g.fillAll(juce::Colour(0xff3c3c3c)); // Alternate row color
}

void MusicLibrary::paintCell(juce::Graphics& g, int rowNumber, int columnId, int width, int height, bool rowIsSelected)
{
    g.setColour(rowIsSelected ? juce::Colours::white : juce::Colours::lightgrey);
    g.setFont(14.0f);

    if (rowNumber < static_cast<int>(tracks.size()))
    {
        juce::String text;
        if (columnId == 1) // Track name
            text = displayedTracks[static_cast<size_t>(rowNumber)].title;
        else if (columnId == 2) // Duration
            text = displayedTracks[static_cast<size_t>(rowNumber)].duration;
        
        g.drawText(text, 2, 0, width - 4, height, juce::Justification::centredLeft, true);
    }
}

bool MusicLibrary::isInterestedInFileDrag(const juce::StringArray& files)
{
    // Check if dragged files are audio files
    for (const auto& file : files)
    {
        if (file.endsWithIgnoreCase(".wav") || 
            file.endsWithIgnoreCase(".mp3") || 
            file.endsWithIgnoreCase(".aif") ||
            file.endsWithIgnoreCase(".flac"))
        {
            return true;
        }
    }
    return false;
}

void MusicLibrary::filesDropped(const juce::StringArray& files, int x, int y)
{
    // Add dropped files to library
    for (const auto& file : files)
    {
        addTrack(file);
    }
}

void MusicLibrary::addTrack(const juce::String& filePath)
{
    Track newTrack;
    newTrack.filePath = filePath;
    newTrack.title = extractTitle(filePath);
    newTrack.duration = getDuration(filePath);
    
    tracks.push_back(newTrack);
    displayedTracks = tracks;
    tableComponent.updateContent();
    
    // Save after adding new track
    saveLibrary();
}

juce::String MusicLibrary::getTrackPath(int index)
{
    if (index >= 0 && index < static_cast<int>(tracks.size()))
        return tracks[static_cast<size_t>(index)].filePath;
    return {};
}

juce::String MusicLibrary::extractTitle(const juce::String& filePath)
{
    // Extract filename without extension as title
    juce::File file(filePath);
    return file.getFileNameWithoutExtension();
}

juce::String MusicLibrary::getDuration(const juce::String& filePath)
{
    juce::File file(filePath);

    // Create a local format manager to read the actual audio file metadata
    juce::AudioFormatManager formatManager;
    formatManager.registerBasicFormats();

    std::unique_ptr<juce::AudioFormatReader> reader(formatManager.createReaderFor(file));

    if (reader != nullptr)
    {
        // Calculate exact real time from samples and sample rate
        double lengthInSeconds = reader->lengthInSamples / reader->sampleRate;
        int minutes = static_cast<int>(lengthInSeconds) / 60;
        int seconds = static_cast<int>(lengthInSeconds) % 60;

        return juce::String::formatted("%d:%02d", minutes, seconds);
    }

    return "0:00"; // Fallback if file is unreadable
}

void MusicLibrary::saveLibrary()
{
    // Save tracks to XML file for persistence
    auto libraryFile = juce::File::getSpecialLocation(juce::File::userApplicationDataDirectory)
                      .getChildFile("Otodecks")
                      .getChildFile("library.xml");
    
    libraryFile.getParentDirectory().createDirectory();
    
    juce::XmlElement library("Library");
    
    for (const auto& track : tracks)
    {
        auto* trackElement = library.createNewChildElement("Track");
        trackElement->setAttribute("title", track.title);
        trackElement->setAttribute("path", track.filePath);
        trackElement->setAttribute("duration", track.duration);
    }
    
    library.writeTo(libraryFile);
}

void MusicLibrary::loadLibrary()
{
    // Load tracks from XML file
    auto libraryFile = juce::File::getSpecialLocation(juce::File::userApplicationDataDirectory)
                      .getChildFile("Otodecks")
                      .getChildFile("library.xml");
    
    if (libraryFile.existsAsFile())
    {
        auto library = juce::XmlDocument::parse(libraryFile);
        if (library != nullptr)
        {
            tracks.clear();
            
            for (auto* trackElement : library->getChildIterator())
            {
                if (trackElement->hasTagName("Track"))
                {
                    Track track;
                    track.title = trackElement->getStringAttribute("title");
                    track.filePath = trackElement->getStringAttribute("path");
                    track.duration = trackElement->getStringAttribute("duration");
                    tracks.push_back(track);
                }
            }
            displayedTracks = tracks;
            tableComponent.updateContent();
        }
    }
}

// Set pointers to deck GUIs (call from parent/owner when decks are available)
void MusicLibrary::setDeckPointers(DeckGUI* deck1, DeckGUI* deck2)
{
    deckPtr1 = deck1;
    deckPtr2 = deck2;
}

// Load a track to a deck by index
void MusicLibrary::loadTrackToDeck(int trackIndex, int deckNumber)
{
    if (trackIndex >= 0 && trackIndex < static_cast<int>(displayedTracks.size()))
    {
        auto trackPath = displayedTracks[static_cast<size_t>(trackIndex)].filePath;
        juce::URL audioURL = juce::URL{ juce::File{ trackPath } };
        
        if (deckNumber == 1 && deckPtr1 != nullptr)
        {
            deckPtr1->loadFile(audioURL);
        }
        else if (deckNumber == 2 && deckPtr2 != nullptr)
        {
            deckPtr2->loadFile(audioURL);
        }
    }
}

void MusicLibrary::searchTracks(const juce::String& searchTerm)
{
    if (searchTerm.isEmpty())
    {
        displayedTracks = tracks;
    }
    else
    {
        displayedTracks.clear();
        for (const auto& track : tracks)
        {
            if (track.title.containsIgnoreCase(searchTerm))
            {
                displayedTracks.push_back(track);
            }
        }
    }
    tableComponent.updateContent();
}

void MusicLibrary::buttonClicked(juce::Button* button)
{
    if (button == &loadButton)
    {
        fileChooser = std::make_unique<juce::FileChooser>(
            "Select files...",
            juce::File::getSpecialLocation(juce::File::userHomeDirectory),
            "*.wav;*.mp3;*.aif;*.flac");

        fileChooser->launchAsync(juce::FileBrowserComponent::openMode
            | juce::FileBrowserComponent::canSelectFiles
            | juce::FileBrowserComponent::canSelectMultipleItems,
            [this](const juce::FileChooser& fc)
            {
                auto results = fc.getResults();
                if (results.isEmpty())
                    return;
                for (auto& file : results)
                    addTrack(file.getFullPathName());
            });
    }
}