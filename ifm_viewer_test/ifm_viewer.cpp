/*
* Copyright (C) 2018 ifm electronic, gmbh
*
* Licensed under the Apache License, Version 2.0 (the "License");
* you may not use this file except in compliance with the License.
* You may obtain a copy of the License at
*
* http://www.apache.org/licenses/LICENSE-2.0
*
* Unless required by applicable law or agreed to in writing, software
* distributed under the License is distribted on an "AS IS" BASIS,
* WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
* See the License for the specific language governing permissions and
* limitations under the License.
*/

#include <cstdint>
#include <exception>
#include <iostream>
#include <memory>
#include <string>
#include <pcl/visualization/cloud_viewer.h>
#include <ifm3d/camera.h>
#include <ifm3d/fg.h>
#include <ifm3d/image.h>

//==============================
// Class wrapping the pcl viewer
//==============================

class IFM3DViewer
{
public:
	IFM3DViewer(ifm3d::Camera::Ptr cam)
		: cam_(cam), description_("ifm3d-pcl-viewer")
	{}

	void Run()
	{
		int win_w = 800;
		int win_h = 600;

		auto fg = std::make_shared<ifm3d::FrameGrabber>(this->cam_, 0xFFFF);
		auto buff = std::make_shared<ifm3d::ImageBuffer>();

		//
		// setup for point cloud
		//
		auto pclvis_ =
			std::make_shared<pcl::visualization::PCLVisualizer>(this->description_);
		pclvis_->setSize(win_w, win_h);

		int v_pcl(0);
		pclvis_->createViewPort(0., 0., 1., 1., v_pcl);
		pclvis_->setBackgroundColor(0, 0, 0, v_pcl);
		pclvis_->setCameraPosition(-3.0, // x-position
			0,    // y-position
			0,    // z-position
			0,    // x-axis "up" (0 = false)
			0,    // y-axis "up" (0 = false)
			1,    // z-axis "up" (1 = true)
			v_pcl);    // viewport

					   // use "A" and "a" to toggle axes indicators
		pclvis_->registerKeyboardCallback(
			[&](const pcl::visualization::KeyboardEvent& ev)
		{
			if (ev.getKeySym() == "A" && ev.keyDown())
			{
				pclvis_->addCoordinateSystem(1., "ifm3d", v_pcl);
			}
			else if (ev.getKeySym() == "a" && ev.keyDown())
			{
				pclvis_->removeCoordinateSystem("ifm3d");
			}
		});

		bool is_first = true;
		while (!pclvis_->wasStopped())
		{
			pclvis_->spinOnce(100);

			if (!fg->WaitForFrame(buff.get(), 500))
			{
				continue;
			}

			//------------
			// Point cloud
			//------------
			pcl::visualization::PointCloudColorHandlerGenericField<ifm3d::PointT>
				color_handler(buff->Cloud(), "intensity");

			if (is_first)
			{
				is_first = false;
				pclvis_->addPointCloud(buff->Cloud(), color_handler, "cloud",
					v_pcl);
			}
			else
			{
				pclvis_->updatePointCloud(buff->Cloud(), color_handler, "cloud");
			}
		} // end: while (...)
	}

private:
	ifm3d::Camera::Ptr cam_;
	std::string description_;

}; // end: IFM3DViewer

   //===================================================
   //===================================================

int main(int argc, const char **argv)
{
	std::string ifm3d_ip;
	std::cout << "Please enter the ip of your camera: " << endl;
	std::cin >> ifm3d_ip;
	try
	{
		//auto cam = ifm3d::Camera::MakeShared();
		ifm3d::Camera::Ptr cam = std::make_shared<ifm3d::Camera>(ifm3d_ip);
		IFM3DViewer viewer(cam);
		viewer.Run();
	}
	catch (const std::exception& ex)
	{
		std::cerr << ex.what() << std::endl;
		return 1;
	}

	return 0;
}