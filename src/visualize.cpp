#include <pcl/ModelCoefficients.h>
#include <pcl/point_cloud.h>
#include <pcl/point_types.h>
#include <pcl/io/pcd_io.h>
#include <pcl/visualization/cloud_viewer.h>
#include <iostream>
#include <pcl/visualization/common/common.h>
#include <pcl/visualization/point_cloud_color_handlers.h>
#include <pcl/visualization/point_picking_event.h>
#include <thread>
#include <mutex>
#include <pcl/common/transforms.h>

#include "PCUtils.h"
#include <vector>

using PointCloudT = pcl::PointCloud<pcl::PointXYZRGB>;

// TODO add multiple input clouds

bool recopy = false;

static bool compare_z(pcl::PointXYZRGB &a, pcl::PointXYZRGB &b)
{
	return (a.z < b.z);
}

static bool compare_x(pcl::PointXYZRGB &a, pcl::PointXYZRGB &b)
{
	return (a.x < b.x);
}

static bool compare_y(pcl::PointXYZRGB &a, pcl::PointXYZRGB &b)
{
	return (a.y < b.y);
}

std::mutex g_mutex;

// thread here
// TODO por alguma razão nuvens RGB (kinect) dão problema
// TODO Implementar Pause, resume ..
// TODO cores !
// TODO velocidade flexivel
void slow_copy(const pcl::PointCloud<pcl::PointXYZRGB>::ConstPtr &src, pcl::PointCloud<pcl::PointXYZRGB>::Ptr tgt, float delay)
{

	std::lock_guard<std::mutex> guard(g_mutex);
	int size = src->size();

	tgt->clear();
	tgt->resize(size);

	std::cout << "animating" << std::endl;
	for (int i = 0; i < size; ++i)
	{
		tgt->points[i] = src->points[i];
		std::this_thread::sleep_for(std::chrono::nanoseconds(1000));
	}

	std::cout << "finished" << std::endl;
	recopy = false;
}

void keyCallback(const pcl::visualization::KeyboardEvent &event)
{
	// std::cout << "key pressed " << std::endl;
	char key = event.getKeyCode();

	if (key == 'l')
	{

		// std::cout << "l" << std::endl;

		if (g_mutex.try_lock() == false)
		{
			//std::cout << "locked" << std::endl;
		}
		else
		{
			// std::cout << "UNlocked" << std::endl;
			g_mutex.unlock();
			recopy = true;
		}
	}
}

void make_grid(pcl::visualization::PCLVisualizer &viewer, float res = 1)
{
	float x_min = -10;
	float x_max = 10;
	float y_min = -10;
	float y_max = 10;
	int N_lines = 10;

	pcl::PointXYZ p1, p2, p3, p4;
	// p1.x = 0;
	// p2.x = 0;
	// p3.x = 0;
	// p4.x = 0;
	// p1.y = 0;
	// p2.y = 0;
	// p3.y = 0;
	// p4.y = 0;
	// p1.z = 0;
	// p2.z = 0;
	// p3.z = 0;
	// p4.z = 0;

	p1.x = x_min;
	p2.x = x_max;
	p1.y = y_min;
	p2.y = y_min;

	p3.x = x_min;
	p4.x = x_min;
	p3.y = y_min;
	p4.y = y_max;

	float y = y_min;
	float x = x_min;
	int i = 0;
	while (y < y_max && x < x_max)
	{
		p1.y = y;
		p2.y = y;
		p3.x = x;
		p4.x = x;
		viewer.addLine(p1, p2, "line" + std::to_string(i++));
		viewer.addLine(p3, p4, "line" + std::to_string(i++));

		y += res;
		x += res;
	}
	p1.y = y;
	p2.y = y;
	p3.x = x;
	p4.x = x;

	viewer.addLine(p1, p2, "line" + std::to_string(i++));
	viewer.addLine(p3, p4, "line" + std::to_string(i++));
}

void pp_callback(const pcl::visualization::PointPickingEvent &event)
{

	if (event.getPointIndex() != -1)
	{

		float x, y, z;
		event.getPoint(x, y, z);
		int i = event.getPointIndex();
		std::cout << "i:" << i << " x = " << x << ";" << y << ";" << z << std::endl;
	}
}

typedef pcl::PointCloud<pcl::PointXYZ> CloudType;

int main(int argc, char **argv)
{

	
	if (argc < 2)
	{
		std::cerr << "Usage : vizualise cloud.pcd/.txt ..." << std::endl;
		exit(-1);
	}

	pcl::visualization::PCLVisualizer viewer("My Viewer");
	int v1, v2;
	viewer.createViewPort(0, 0, 1, 1, v1);
	// viewer.setCameraPosition(-6.61, -2.13, 10.33, 1.0, 1, 1);
	// viewer.setBackgroundColor(0, 0, 0, v1);
	viewer.addCoordinateSystem(1, "ref", v1);
	viewer.registerPointPickingCallback(pp_callback);
	viewer.registerKeyboardCallback(keyCallback);
	make_grid(viewer, 1);

	
	int n_clouds = argc - 1;
	
	std::cout << "N clouds = " << n_clouds << std::endl;

	std::vector<PointCloudT::Ptr> cloud_vector(n_clouds);


	for (int i = 0; i < n_clouds; i++)
	{
		std::cout << "Reading file .. " << i << std::endl;
		cloud_vector[i] = pcl::make_shared<PointCloudT>();
		
		PCUtils::readFile(argv[i+1], *cloud_vector[i]);
		pcl::visualization::PointCloudColorHandlerRandom<pcl::PointXYZRGB> random_color(cloud_vector[i]);
		viewer.addPointCloud(cloud_vector[i],random_color,"cloud" + std::to_string(i));
	}

	std::cout << "Done" << std::endl;

	while (!viewer.wasStopped())
	{
		viewer.spin();
	}

	return 0;
}
