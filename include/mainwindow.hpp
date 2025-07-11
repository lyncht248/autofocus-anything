#ifndef HVIGTK_MAINWINDOW_H
#define HVIGTK_MAINWINDOW_H

#include <gtkmm.h>
#include <cairomm/cairomm.h>
#include <unordered_map>

#include "cond.hpp"
#include "gtkmm/drawingarea.h"
#include "sigc++/connection.h"
#include "vidframe.hpp"
#include "sharpness_graph.hpp"

class ScaleWidget : public Gtk::Bin
{
public:
	ScaleWidget(double lower, double upper, double inc, double def, int spinButtonWidth = 0, int scaleWidth = 0, bool stepSnap = false);
	virtual ~ScaleWidget();

	using SignalChanged = sigc::signal<void(double)>;
	SignalChanged signalChanged();

	void setSpinButtonPrec(int digits);
	void setSpinButtonWidth(int width);
	void setScaleSizeRequest(int width, int height);

	double getValue() const;
	void setValue(double v);
	void setUpperLimit(double value);

private:
	void spinButtonChanged();
	void scaleChanged();

	SignalChanged sigChanged;

	Gtk::Scale scale;
	Gtk::SpinButton spinButton;
	Gtk::Grid grid;

	sigc::connection spinButtonConnection, scaleConnection;
	bool stepSnap;
};

class RenderFilter
{
public:
	virtual void draw(const ::Cairo::RefPtr<::Cairo::Context> &cr) = 0;
};

/*
This class is responsible for the GTK GUI buttons, logic, etc., and is a derived class from GTK::Window
*/
class MainWindow : public Gtk::Window
{
public:
	struct Private;
	MainWindow();
	virtual ~MainWindow();

	// double getFrameRateScaleValue() const;
	double getStabWaitScaleValue() const;
	double getRecordingSizeScaleValue() const;
	double getBestFocusScaleValue() const;

	void setBestFocusScaleValue(double v);

	void getDisplayDimensions(double &w, double &h) const;

	void updateCameraValues(double gain, double expose, double gamma);

	void displayMessageFPS(const std::string &msg);
	void displayMessageLoadSave(const std::string &msg);
	void displayMessageError(const std::string &msg);

	void displayWarningMessage(const std::string &msg);

	void renderFrame(VidFrame *frame);

	using SignalFrameDrawn = sigc::signal<void()>;
	SignalFrameDrawn signalFrameDrawn();

	using SignalFeatureUpdated = sigc::signal<void(std::string, double)>;
	SignalFeatureUpdated signalFeatureUpdated();

	// using SignalThresholdChanged = sigc::signal<void(double)>;
	// SignalThresholdChanged signalThresholdChanged();

	// using SignalScaleChanged = sigc::signal<void(double)>;
	// SignalScaleChanged signalScaleChanged();

	using SignalBestFocusChanged = sigc::signal<void(double)>;
	SignalBestFocusChanged signalBestFocusChanged();

	using SignalPauseClicked = sigc::signal<void()>;
	SignalPauseClicked signalPauseClicked();

	using SignalEnterClicked = sigc::signal<void()>;
	SignalEnterClicked signalEnterClicked();

	using SignalFindFocusClicked = sigc::signal<void()>;
	SignalFindFocusClicked signalFindFocusClicked();

	using SignalResetClicked = sigc::signal<void()>;
	SignalResetClicked signalResetClicked();

	using SignalRecenterClicked = sigc::signal<void()>;
	SignalRecenterClicked signalRecenterClicked();

	using SignalGetDepthsClicked = sigc::signal<void()>;
	SignalGetDepthsClicked signalGetDepthsClicked();

	void setHasBuffer(bool val);
	void setLiveView(bool val);
	void setLoading(bool val);
	void setSaving(bool val);
	void setPlayingBuffer(bool val);
	void setSeeking(bool val);
	void setRecording(bool val);
	void setMakingMap(bool val);
	void setShowingMap(bool val);
	void set3DStab(bool val);
	void setHoldFocus(bool val);
	void setFindFocus(bool val);
	void setStabiliseActive(bool val);
	void setViewDepths(bool val);

	// Condition& getMakeMapActive(); DEPRECATED
	Condition &getStabiliseActive();
	// Condition& getShowMapActive(); DEPRECATED
	Condition &getHoldFocusActive();
	Condition &get3DStabActive();
	Condition &get2DStabActive();
	Condition &getHasBuffer();
	Condition &getLiveView();
	Condition &getLoading();
	Condition &getSaving();
	Condition &getPlayingBuffer();
	Condition &getSeeking();
	Condition &getRecording();
	Condition &getPausedRecording();
	Condition &getFindFocusActive();
	Condition &getViewDepthsActive();
	Condition &getGetDepthsActive();

	int getFrameSliderValue() const;
	double getFrameRateEntryBox() const;
	std::string getFileLocation() const;

	void addRenderFilter(const std::string &key, RenderFilter *filter);
	RenderFilter *removeRenderFilter(const std::string &key);

	void setTrackingFPS(bool val);

	// CONCERNING
	void showOutOfBoundsWarning();
	void hideOutOfBoundsWarning();

	using SignalHomePositionChanged = sigc::signal<void(double)>;
	SignalHomePositionChanged signalHomePositionChanged();

	using SignalPGainChanged = sigc::signal<void(double)>;
	SignalPGainChanged signalPGainChanged();

	void updateSharpnessGraph(const std::vector<double> &values);

protected:
	virtual void on_realize() override;
	virtual void on_show() override;

	bool _on_state_event(GdkEventWindowState *window_state_event);

private:
	const std::string DEFAULT_HVI_PATH = "/home/hvi/Desktop/HVI-data";
	std::string current_file_path = DEFAULT_HVI_PATH;

	/*
	 * This is the function which will draw the currently loaded frame.
	 * Currently the Cairo library is used. To trigger this function
	 * call MainWindow::renderFrame(...).
	 */
	bool renderDisplay(const ::Cairo::RefPtr<::Cairo::Context> &cr);

	void onFrameDrawn();
	bool updateFPSCounter();

	// void whenMakeMapToggled(bool makingMap); DEPRECATED
	void whenStabiliseToggled(bool stabilising);
	// void whenShowMapToggled(bool showingMap); DEPRECATED

	void whenHoldFocusToggled(bool holdingFocus);
	void when3DStabToggled(bool active);
	void when2DStabToggled(bool active2);
	void onFindFocusClicked();
	void onResetClicked();
	void onRecenterClicked();

	void bufferFilled();
	void bufferEmptied();

	void viewingLive();
	void viewingBuffer();

	void whenLoadingToggled(bool loading);
	void whenSavingToggled(bool saving);
	void whenRecordingToggled(bool recording);
	void whenPausedRecordingToggled(bool paused);
	void whenTrackingFPSToggled(bool tracking);

	void onLoadButtonClicked();
	void onSaveButtonClicked();

	void onPlayButtonClicked();
	void onLiveToggled();
	void onRecordClicked();
	void onPauseClicked();
	void onBackButtonClicked();

	void whenPlayingBufferToggled(bool playing);

	void onFrameSliderChange(double val);

	void onGainScaleChange(double val);
	void onExposeScaleChange(double val);
	void onGammaScaleChange(double val);
	void onFrameRateChange(double val);

	// void onThresScaleChange(double val); DEPRECATED
	// void onScaleScaleChange(double val); DEPRECATED
	void onRecordingSizeScaleChange(double val);
	void onBestFocusScaleChange(double val);
	void onHomePositionScaleChange(double val);
	void onPGainScaleChange(double val);
	// Add the missing function declaration for handling findFocusToggle
	void onFindFocusToggled();

	void onGetDepthsClicked();
	void whenGetDepthsToggled(bool gettingDepths);
	void whenViewDepthsToggled(bool viewingDepths);

	struct Private *priv;

	ScaleWidget gainScale, exposeScale, gammaScale, frameSlider, waitScale, recordingSizeScale, bestFocusScale, homePositionScale, pGainScale;
	Gtk::Button recordButton, backToStartButton, pauseButton, playButton, fileLoadButton, fileSaveButton, resetButton, enterButton, recenterButton;
	Gtk::ToggleButton getDepthsToggle;
	Gtk::ToggleButton liveToggle, makeMapToggle, stabiliseToggle, showMapToggle, holdFocusToggle, threedStabToggle, twodStabToggle, findFocusToggle, viewDepthsToggle;
	Gtk::Label fpsLabel, loadSaveLabel, errorLabel, outOfBoundsWarningLabel;
	Gtk::Entry frameRateEntry;

	VidFrame *drawFrame;
	bool newDrawFrame;
	int countFrames;

	Condition makeMapActive, stabiliseActive, showMapActive, holdFocusActive, threedStabActive, twodStabActive,
		hasBuffer, liveView, loading, saving, playingBuffer, seeking, recording, pausedRecording, trackingFPS, viewDepthsActive, getDepthsActive;

	SignalFrameDrawn sigFrameDrawn;
	SignalFeatureUpdated sigFeatureUpdated;
	// SignalThresholdChanged sigThresholdChanged;
	// SignalScaleChanged sigScaleChanged;
	SignalBestFocusChanged sigBestFocusChanged;
	SignalPauseClicked sigPauseClicked;
	SignalEnterClicked sigEnterClicked;
	SignalFindFocusClicked sigFindFocusClicked;
	SignalResetClicked sigResetClicked;
	SignalRecenterClicked sigRecenterClicked;
	SignalGetDepthsClicked sigGetDepthsClicked;
	sigc::connection gainScaleConnection, exposeScaleConnection, gammaScaleConnection, frameRateScaleConnection, frameSliderConnection, stateChangeConnection;

	std::unordered_map<std::string, RenderFilter *> renderFilters;

	SignalHomePositionChanged sigHomePositionChanged;
	sigc::connection homePositionScaleConnection;

	SignalPGainChanged sigPGainChanged;
	sigc::connection pGainScaleConnection;

	Gtk::Label sharpnessLabel;
	SharpnessGraph sharpnessGraph;
};

#endif
