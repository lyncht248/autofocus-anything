#ifndef HVIGTK_SYSTEM_H
#define HVIGTK_SYSTEM_H

#include <fstream>
#include <iostream>
#include <queue>

#include "cairomm/surface.h"
#include "vidframe.hpp"
#include "mainwindow.hpp"
#include "autofocus.hpp"
#include "cond.hpp"
#include "thread.hpp"
#include "recorder.hpp"
#include "tsqueue.hpp"
#include "logfile.hpp"
#include "stabiliser.hpp"
#include "framefilter.hpp"
#include "phasecorr2_stabiliser.hpp"
#include "sharpness_analyzer.hpp"
#include "sharpness_graph.hpp"
#include "imagingcam.hpp"
#include "sdlwincommon.hpp"

namespace Vimba = AVT::VmbAPI;

class System;

class FrameProcessor
{
	friend class System;

public:
	FrameProcessor(System &sys);
	~FrameProcessor();

	void releaseFrame();
	void resetRaster();
	::Cairo::RefPtr<::Cairo::Surface> getFrame();
	void resetZoom();

protected: // Only available to derived and friend classes
	void stabilise();
	void processFrame();

	::Cairo::RefPtr<::Cairo::ImageSurface> processed;
	VidFrame *vidFrame;
	std::unordered_map<std::string, FrameFilter *> filters;
	bool running;
	bool _stabNewFrame;
	bool _stabComplete;
	bool _frameReleased;

	Glib::Threads::Mutex mutex, stabMutex;
	Glib::Threads::Cond frameReleased, stabNewFrame, stabComplete;

	System &system;

	TooN::Vector<2> offset, currentOff, rasterPos;
	TSQueue<VidFrame *> stabQueue, released;

	Glib::Threads::Thread *processorThread, *stabThread;
};

/*
This class is responsible for all the processing behind the GUI, including frame streaming (see FrameProcessor), closing/opening the GUI,
and operating other neccessary threads.
*/
class System
{
	friend class FrameProcessor; // Allows FrameProcessor to access private methods
public:
	// Constructor function for system class
	System(int argc, char **argv);
	~System();

	MainWindow &getWindow();
	Recorder &getRecorder();

	bool startStreaming();
	void stopStreaming();

	template <typename T>
	bool setFeature(const std::string &fname, T val)
	{
		if (cam != nullptr)
		{
			Vimba::FeaturePtr pFeature;
			if (cam->GetFeatureByName(fname.c_str(), pFeature) == VmbErrorSuccess && pFeature->SetValue(val) == VmbErrorSuccess)
			{
				logger->info("[System.hpp] Set {} to: {}", fname, val);
				return true;
			}
			else
			{
				logger->error("[System.hpp] FAILED to set {} to: {}", fname, val);
				return false;
			}
		}
		return false;
	}

	template <typename T>
	T getFeature(const std::string &fname)
	{
		if (cam != nullptr)
		{
			Vimba::FeaturePtr pFeature;
			if (cam->GetFeatureByName(fname.c_str(), pFeature) == VmbErrorSuccess)
			{
				T out;
				pFeature->GetValue(out);
				logger->info("[System.hpp] Got {} = {}", fname, out);
				return out;
			}
			else
			{
				logger->error("[System.hpp] FAILED to get {}", fname);
			}
		}

		return T();
	}

	TSQueue<VidFrame *> &getFrameQueue();
	Glib::Dispatcher &signalNewFrame();

	VidFrame *getFrame();
	double getFPS(); // see TODO below

	void onWindowHomePositionChanged(double val);
	void onWindowPGainChanged(double val);

	// Method to update ROI center from SDL window
	void updateROICenter(int x, int y);

	// Method to clear ROI display from SDL window
	void clearROIDisplay();

	// Get current ROI info for display
	void getCurrentROI(int &centerX, int &centerY, int &width, int &height) const;

	// Method to set SDL window reference for ROI updates
	void setSDLWindow(SDLWindow::SDLWin *win);

	// Get access to imaging camera
	ImagingCam *getImagingCam() { return imagingCam.get(); }

	// Get current stabilization offset (returns true if stabilization is active)
	bool getStabilizationOffset(double &offsetX, double &offsetY);

	void onGetDepthsClicked();
	void whenGetDepthsToggled(bool gettingDepths);
	void whenViewDepthsToggled(bool viewingDepths);

	// Depth mapping data structures
	struct DepthMapData
	{
		std::vector<std::vector<std::pair<double, double>>> depthImage; // max_sharpness, actual_focus_position pairs
		int width;
		int height;
		bool isValid;

		DepthMapData() : width(0), height(0), isValid(false) {}
	};

	DepthMapData currentDepthMap;

private:
	// Helper method to create stabilizer map with default parameters
	void createStabiliserMapWithDefaults(const VidFrame &frame);

	// Helper method to calculate threshold for target percentage of pixels
	double calculateThresholdForPercentage(const CVD::Image<unsigned char> &image, double vesselSize, double targetPercentage);

	void renderFrame();
	void releaseFrame();

	void whenLiveViewToggled(bool viewingLive);

	// void whenMakeMapToggled(bool makingMap); DEPRECATED
	void whenStabiliseToggled(bool stabilising);
	// void whenShowMapToggled(bool showingMap); DEPRECATED

	void whenHoldFocusToggled(bool holdingFocus);
	void when3DStabToggled(bool active);
	void when2DStabToggled(bool active2);

	void whenLoadingToggled(bool loading);
	void whenSavingToggled(bool saving);
	void whenPlayingToggled(bool playing);
	void whenSeekingToggled(bool seeking);
	void whenRecordingToggled(bool recording);

	void onRecorderOperationComplete(RecOpRes res);
	void onWindowFeatureUpdated(std::string fname, double val);

	void onWindowThresholdChanged(double val);
	void onWindowScaleChanged(double val);
	void onWindowBestFocusChanged(double val);
	void onWindowPauseClicked();
	void onFindFocusClicked();
	void onResetClicked();
	void onRecenterClicked();
	void onWindowEnterClicked();

	bool onCloseClicked(const GdkEventAny *event);

	void on_error();
	void handleDisconnection();

	struct Private;
	MainWindow window;		  // object from mainwindow.cpp class
	autofocus AF;			  // object from autofocus.cpp class
	Vimba::VimbaSystem &vsys; // For interacting with Vimba camera
	Vimba::CameraPtr cam;	  // For interacting with Vimba camera

	CVD::ImageRef size;
	CVD::Image<unsigned int> im;

	struct Private *priv;

	Glib::Dispatcher sigNewFrame;

	TSQueue<VidFrame *> frameQueue;

	FrameProcessor frameProcessor;

	ObjectThread thread;

	// Recorder *recorder;
	std::unique_ptr<Recorder> recorder; // Attempting to fix some memory leakage
	VDispatcher<RecOpRes> sRecorderOperationComplete;

	Stabiliser stabiliser; // For running x-y stabilization
	bool madeMap;
	bool errorDialogShown = false;
	// double actualFPS; //TODO: Implement actualFPS instead of using value in box

	// Flags for disconnection states
	bool tiltedCamDisconnected = false;
	bool imagingCamDisconnected = false;
	bool lensDisconnected = false;

	// Add a boolean to enable or disable PhaseCorr usage.
	bool usePhaseCorr = true;

	// Add a PhaseCorrStabiliser2 object
	PhaseCorrStabiliser2 phaseCorrStabiliser{8, 1.0}; // 1.0 = complete replacement like old stabilizer
	// Add a boolean to check if the reference frame is set
	bool phaseCorrStabiliserReferenceNotSet = true;

	// Sharpness analysis components
	SharpnessAnalyzer sharpnessAnalyzer;
	Glib::Dispatcher sigSharpnessUpdated;
	std::vector<double> currentSharpnessValues;
	bool sharpnessUpdateEnabled;
	std::chrono::time_point<std::chrono::steady_clock> lastSharpnessUpdate;
	SharpnessGraph sharpnessGraph;

	// Method to update the sharpness graph in the UI
	void updateSharpnessGraph();

	// Imaging camera for ROI-based autofocus
	std::unique_ptr<ImagingCam> imagingCam;

	// Flag to prevent clearing ROI when imaging camera updates best focus
	bool imagingCamUpdatingFocus = false;
};

#endif
