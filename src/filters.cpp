#include <pcl/filters/grid_minimum.h>
#include <pcl/filters/voxel_grid.h>
#include <pcl/point_types.h>
#include <pcl/io/pcd_io.h>
#include <iostream>

#include <unistd.h>

#include <pcl/filters/statistical_outlier_removal.h>
#include <pcl/visualization/cloud_viewer.h>

#include "PCUtils.h"

typedef pcl::PointCloud<pcl::PointXYZRGB> CloudType;

int main(int argc, char **argv)
{

	CloudType::Ptr cloud(new CloudType);

	if (argc < 5)
	{
		std::cerr << "Usage : filters cloud.pcd [res] [sor neighboorhood] [sor deviation]" << std::endl;
		std::cerr << "Recommended values : res = 0.1, sor neighboorhood = 50, sor deviation = 1" << std::endl;
		exit(-1);
	}

	std::cout << "Loading input point clouds..." << std::endl;
	if (pcl::io::loadPCDFile(argv[1], *cloud) != 0)
		exit(-1);

	std::cout << "Loaded: " << cloud->size() << " points." << std::endl;
	// cloudname
	std::string arg(argv[1]);
	std::string grid_cloudname, voxel_cloudname, sor_cloudname;
	int slash = arg.find_last_of('/'); //path
	int dot = arg.find_last_of('.');   //extension
	grid_cloudname = arg.substr(slash + 1, dot - slash - 1) + "_" + "gridded.pcd";
	voxel_cloudname = arg.substr(slash + 1, dot - slash - 1) + "_" + "voxeled.pcd";
	sor_cloudname = arg.substr(slash + 1, dot - slash - 1) + "_" + "sored.pcd";

	CloudType::Ptr cloud_filtered(new CloudType);
	CloudType::Ptr cloud_filtered2(new CloudType); //Voxel Grid
	CloudType::Ptr cloud_filtered3(new CloudType); // SOR Filter

	clock_t start = start,end;

	float res = atof(argv[2]);
	std::cout << "res = " << res << std::endl;
	start = clock();
	std::cout << "Computing Grid Minimum..." << std::endl;
	pcl::GridMinimum<pcl::PointXYZRGB> grid(res); //resolution
	grid.setInputCloud(cloud);
	grid.filter(*cloud_filtered);
	end = clock();
	std::cout << "OK! Number of points: " << cloud_filtered->size() << "( " << (float)(end-start)/CLOCKS_PER_SEC << " s)" << std::endl;
	//
	
	start = clock();
	std::cout << "Computing Voxel Grid..." << std::endl;
	pcl::VoxelGrid<pcl::PointXYZRGB> voxel;
	voxel.setInputCloud(cloud);
	voxel.setLeafSize(res, res, res);
	voxel.filter(*cloud_filtered2);
	end = clock();
	std::cout << "OK! Number of points: " << cloud_filtered2->size() << "( " << (float)(end-start)/CLOCKS_PER_SEC << " s)" << std::endl;
	//
	// MeanK = 200, StdThresh = 5 for stockpile
	start = clock();
	std::cout << "Computing SOR..." << std::endl;
	pcl::StatisticalOutlierRemoval<pcl::PointXYZRGB> sor;
	sor.setInputCloud(cloud);
	sor.setMeanK(atof(argv[3]));
	sor.setStddevMulThresh(atof(argv[4]));
	sor.filter(*cloud_filtered3);
	end = clock();
	std::cout << "OK! Number of points: " << cloud_filtered3->size() << "( " << (float)(end-start)/CLOCKS_PER_SEC << " s)" << std::endl;

	//Paint pointClouds
	int max = cloud->size();
	for (int i = 0; i < max; ++i)
	{

		PCUtils::setColorMap(i, max, cloud->points[i]);
	}
	max = cloud_filtered2->size();
	for (int i = 0; i < max; ++i)
	{
		PCUtils::setColorMap(i, max, cloud_filtered2->points[i]);
	}
	max = cloud_filtered3->size();
	for (int i = 0; i < max; ++i)
	{
		PCUtils::setColorMap(i, max, cloud_filtered3->points[i]);
	}
	// 		pcl::visualization::PointCloudColorHandlerRGBField<pcl::PointXYZRGB> height_color0(cloud);
	// 		pcl::visualization::PointCloudColorHandlerRGBField<pcl::PointXYZRGB> height_color1(cloud_filtered2);
	// 		pcl::visualization::PointCloudColorHandlerRGBField<pcl::PointXYZRGB> height_color2(cloud_filtered3);

	std::cout << "Wrting output clouds..." << std::endl;
	pcl::io::savePCDFileBinary(grid_cloudname, *cloud_filtered);
	PCL_INFO("subsampled cloud '%s' saved successfully \n", grid_cloudname.c_str());
	pcl::io::savePCDFileBinary(voxel_cloudname, *cloud_filtered2);
	PCL_INFO("subsampled cloud '%s' saved successfully \n", voxel_cloudname.c_str());
	pcl::io::savePCDFileBinary(sor_cloudname, *cloud_filtered3);
	PCL_INFO("subsampled cloud '%s' saved successfully \n", sor_cloudname.c_str());

	char opt;
	bool visualize = false;
	while ((opt = getopt(argc, argv, "v")) != -1)
	{

		if (opt == 'v')
			visualize = true;
	}

	if (visualize)
	{
		int v0, v1, v2;

		std::cout << "Opening Visualizer" << std::endl;
		pcl::visualization::PCLVisualizer::Ptr viewer = pcl::make_shared<pcl::visualization::PCLVisualizer>("Viewer");

		viewer->createViewPort(0, 0, 1, 0.5, v0);
		viewer->createViewPort(0, 0.5, 0.5, 1, v1);
		viewer->createViewPort(0.5, 0.5, 1, 1, v2);
		viewer->addPointCloud<pcl::PointXYZRGB>(cloud, "cloud", v0);
		viewer->addPointCloud<pcl::PointXYZRGB>(cloud_filtered2, "voxel", v1);
		viewer->addPointCloud<pcl::PointXYZRGB>(cloud_filtered3, "sor", v2);

		viewer->addCoordinateSystem(1, "origin");

		while (!viewer->wasStopped())
		{

			viewer->spin();
		}
	}

	return 0;
}
