//Chai Ming Shi Bryan
#pragma once
#include "../JuceLibraryCode/JuceHeader.h"
using namespace juce;
#include <vector>
#include <string>
#include <memory>

// Forward declaration instead of include to avoid circular dependency
class DeckGUI;

/**
 * MusicLibrary class manages a collection of audio tracks
 * Handles loading, storing, and persisting music files
 */
class MusicLibrary : public juce::Component,
                     public juce::TableListBoxModel,
                     public juce::FileDragAndDropTarget,
                     public juce::Button::Listener
{
public:
    MusicLibrary();
    ~MusicLibrary();

    // Component overrides
    void paint(juce::Graphics& g) override;
    void resized() override;

    // TableListBoxModel overrides
    /**
     * Returns the number of rows in the table
     * @return int number of tracks in library
     */
    int getNumRows() override;

    /**
     * Paints a table cell with track information
     * @param g Graphics context
     * @param rowNumber Row index
     * @param columnId Column identifier
     * @param width Cell width
     * @param height Cell height
     * @param rowIsSelected Whether row is selected
     */
    void paintRowBackground(juce::Graphics& g, int rowNumber, int width, int height, bool rowIsSelected) override;
    void paintCell(juce::Graphics& g, int rowNumber, int columnId, int width, int height, bool rowIsSelected) override;

    // FileDragAndDropTarget overrides
    bool isInterestedInFileDrag(const juce::StringArray& files) override;
    void filesDropped(const juce::StringArray& files, int x, int y) override;

    /**
     * Adds a track to the music library
     * @param filePath Path to the audio file
     */
    void addTrack(const juce::String& filePath);

    /**
     * Gets the file path of a track at specified index
     * @param index Track index
     * @return String file path
     */
    juce::String getTrackPath(int index);

    /**
     * Saves library state to file
     */
    void saveLibrary();

    /**
     * Loads library state from file
     */
    void loadLibrary();

    // New methods for deck integration:
    void setDeckPointers(DeckGUI* deck1,DeckGUI* deck2);
    void loadTrackToDeck(int trackIndex, int deckNumber);
    void searchTracks(const juce::String& searchTerm);

    void buttonClicked(juce::Button* button) override;

private:
    // Track info structure
    struct Track
    {
        juce::String title;
        juce::String filePath;
        juce::String duration;
    };

    std::vector<Track> tracks;
    std::vector<Track> displayedTracks;
    juce::TableListBox tableComponent;
    juce::TextButton loadButton;
    juce::TextButton removeButton;
    juce::Label titleLabel;
    juce::TextEditor searchBox;

    // Deck pointers
    DeckGUI* deckPtr1;
    DeckGUI* deckPtr2;
    
    // Deck loading buttons - initialize in constructor, not here
    juce::TextButton loadToDeck1Button;
    juce::TextButton loadToDeck2Button;
    
    int selectedRow;

    // File chooser for opening files
    std::unique_ptr<juce::FileChooser> fileChooser;

    /**
     * Extracts track title from file path
     * @param filePath Full path to file
     * @return String track title
     */
    juce::String extractTitle(const juce::String& filePath);

    /**
     * Gets duration of audio file
     * @param filePath Path to audio file
     * @return String formatted duration
     */
    juce::String getDuration(const juce::String& filePath);

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(MusicLibrary)
};