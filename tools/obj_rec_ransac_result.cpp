/*
 * Software License Agreement (BSD License)
 *
 *  Copyright (c) 2011, Willow Garage, Inc.
 *  All rights reserved.
 *
 *  Redistribution and use in source and binary forms, with or without
 *  modification, are permitted provided that the following conditions
 *  are met:
 *
 *   * Redistributions of source code must retain the above copyright
 *     notice, this list of conditions and the following disclaimer.
 *   * Redistributions in binary form must reproduce the above
 *     copyright notice, this list of conditions and the following
 *     disclaimer in the documentation and/or other materials provided
 *     with the distribution.
 *   * Neither the name of the copyright holder(s) nor the names of its
 *     contributors may be used to endorse or promote products derived
 *     from this software without specific prior written permission.
 *
 *  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 *  "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 *  LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
 *  FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE
 *  COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
 *  INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING,
 *  BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 *  LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
 *  CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 *  LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN
 *  ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 *  POSSIBILITY OF SUCH DAMAGE.
 *
 */

/*
 * obj_rec_ransac_result.cpp
 *
 *  Created on: Jan 23, 2013
 *      Author: papazov
 *
 *  Visualizes the result of the ObjRecRANSAC class. STILL WORK IN PROGRESS!!
 */

#include <pcl/recognition/ransac_based/obj_rec_ransac.h>
#include <pcl/visualization/pcl_visualizer.h>
#include <pcl/console/print.h>
#include <pcl/console/parse.h>
#include <pcl/io/pcd_io.h>
#include <pcl/point_cloud.h>
#include <vtkPolyDataReader.h>
#include <vtkDoubleArray.h>
#include <vtkDataArray.h>
#include <vtkPointData.h>
#include <vtkHedgeHog.h>
#include <cstdio>
#include <vector>
#include <list>

using namespace std;
using namespace pcl;
using namespace io;
using namespace console;
using namespace recognition;
using namespace visualization;

class CallbackParameters;

void keyboardCB (const pcl::visualization::KeyboardEvent &event, void* params_void);
void update (CallbackParameters* params);
bool vtk2PointCloud (const char* file_name, PointCloud<PointXYZ>& pcl_points, PointCloud<Normal>& pcl_normals, vtkPolyData* vtk_data);
void run (float pair_width, float voxel_size, float max_coplanarity_angle);

//#define _SHOW_SCENE_POINTS_
#define _SHOW_OCTREE_POINTS_
//#define _SHOW_OCTREE_NORMALS_
//#define _SHOW_OCTREE_

class CallbackParameters
{
  public:
    CallbackParameters (ObjRecRANSAC& objrec, PCLVisualizer& viz, PointCloud<PointXYZ>& scene_points, PointCloud<Normal>& scene_normals)
    : objrec_ (objrec),
      viz_ (viz),
      scene_points_ (scene_points),
      scene_normals_ (scene_normals)
    {}

    ObjRecRANSAC& objrec_;
    PCLVisualizer& viz_;
    PointCloud<PointXYZ>& scene_points_;
    PointCloud<Normal>& scene_normals_;
    list<vtkActor*> actors_;
};

//===========================================================================================================================================

int
main (int argc, char** argv)
{
  // THIS IS STILL WORK IN PROGRESS!!

  printf ("\nUsage: ./pcl_obj_rec_ransac_scene_opps <pair_width> <voxel_size> <max_coplanarity_angle>\n\n");

  const int num_params = 3;
  float parameters[num_params] = {40.0f/*pair width*/, 5.0f/*voxel size*/, 15.0f/*max co-planarity angle*/};
  string parameter_names[num_params] = {"pair_width", "voxel_size", "max_coplanarity_angle"};

  // Read the user input if any
  for ( int i = 0 ; i < argc-1 && i < num_params ; ++i )
  {
    parameters[i] = static_cast<float> (atof (argv[i+1]));
    if ( parameters[i] <= 0.0f )
    {
      fprintf(stderr, "ERROR: the %i-th parameter has to be positive and not %f\n", i+1, parameters[i]);
      return (-1);
    }
  }

  printf ("The following parameter values will be used:\n");
  for ( int i = 0 ; i < num_params ; ++i )
    cout << "  " << parameter_names[i] << " = " << parameters[i] << endl;
  cout << endl;

  run (parameters[0], parameters[1], parameters[2]);
}

//===========================================================================================================================================

void
run (float pair_width, float voxel_size, float max_coplanarity_angle)
{
  // THIS IS STILL WORK IN PROGRESS!!

  // The input to the object recognition instance
  char scene_file_name[] = "../../test/tum_table_scene.vtk\0", model_file_name[] = "../../test/tum_amicelli_box.vtk\0";
  PointCloud<PointXYZ>::Ptr scene_points (new PointCloud<PointXYZ> ()), amicelli_points (new PointCloud<PointXYZ> ());
  PointCloud<Normal>::Ptr scene_normals (new PointCloud<Normal> ()), amicelli_normals (new PointCloud<Normal> ());

  vtkSmartPointer<vtkPolyData> amicelli_model = vtkSmartPointer<vtkPolyData>::New ();
  string amicelli_name = "amicelli";

  // Get the points and normals from the input model
  if ( !vtk2PointCloud (model_file_name, *amicelli_points, *amicelli_normals, amicelli_model) )
    return;

  // Get the points and normals from the input scene
  if ( !vtk2PointCloud (scene_file_name, *scene_points, *scene_normals, NULL) )
    return;

  // The recognition object
  ObjRecRANSAC objrec (pair_width, voxel_size);
  objrec.setMaxCoplanarityAngleDegrees (max_coplanarity_angle);
  // Add a model
  objrec.addModel (*amicelli_points, *amicelli_normals, amicelli_name, amicelli_model);

  // The parameters for the callback function and the visualizer
  PCLVisualizer viz;
  CallbackParameters params(objrec, viz, *scene_points, *scene_normals);
  viz.registerKeyboardCallback (keyboardCB, static_cast<void*> (&params));

  // Run the recognition and update the viewer. Have a look at this method, to see how to start the recognition and use the result!
  update (&params);

  // From this line on: visualization stuff only!
#ifdef _SHOW_OCTREE_
  show_octree(objrec.getSceneOctree (), viz);
#endif

#ifdef _SHOW_SCENE_POINTS_
  viz.addPointCloud (scene_points, "scene points");
  viz.setPointCloudRenderingProperties (pcl::visualization::PCL_VISUALIZER_POINT_SIZE, 2, "scene points");
#endif

#ifdef _SHOW_OCTREE_POINTS_
  PointCloud<PointXYZ>::Ptr octree_points (new PointCloud<PointXYZ> ());
  objrec.getSceneOctree ().getFullLeafPoints (*octree_points);
  viz.addPointCloud (octree_points, "octree points");
  viz.setPointCloudRenderingProperties (pcl::visualization::PCL_VISUALIZER_POINT_SIZE, 5, "octree points");
  viz.setPointCloudRenderingProperties (pcl::visualization::PCL_VISUALIZER_COLOR, 1.0, 0.0, 0.0, "octree points");
#endif

#if defined _SHOW_OCTREE_NORMALS_ && defined _SHOW_OCTREE_POINTS_
  PointCloud<Normal>::Ptr normals_octree (new PointCloud<Normal> ());
  objrec.getSceneOctree ().getNormalsOfFullLeaves (*normals_octree);
  viz.addPointCloudNormals<PointXYZ,Normal> (points_octree, normals_octree, 1, 6.0f, "normals out");
#endif

  // Enter the main loop
  while (!viz.wasStopped ())
  {
    //main loop of the visualizer
    viz.spinOnce (100);
    boost::this_thread::sleep (boost::posix_time::microseconds (100000));
  }
}

//===============================================================================================================================

void
keyboardCB (const pcl::visualization::KeyboardEvent &event, void* params_void)
{
  if (event.getKeyCode () == 13 /*enter*/ && event.keyUp ())
    update (static_cast<CallbackParameters*> (params_void));
}

//===============================================================================================================================

void
update (CallbackParameters* params)
{
  // Clear the visualizer from old object instances
  vtkRenderer *renderer = params->viz_.getRenderWindow ()->GetRenderers ()->GetFirstRenderer ();
  for ( list<vtkActor*>::iterator it = params->actors_.begin () ; it != params->actors_.end () ; ++it )
    renderer->RemoveActor (*it);
  params->actors_.clear ();

  // This will be the output of the recognition
  list<ObjRecRANSAC::Output> rec_output;

  // For convenience
  ObjRecRANSAC& objrec = params->objrec_;

  // Run the recognition method
  objrec.recognize (params->scene_points_, params->scene_normals_, rec_output);
  int i = 0;

  // Show the hypotheses
  for ( list<ObjRecRANSAC::Output>::iterator it = rec_output.begin () ; it != rec_output.end () ; ++it, ++i )
  {
    cout << it->object_name_ << " has a confidence value of " << it->match_confidence_ << endl;

    // Make a copy of the VTK model
    vtkSmartPointer<vtkPolyData> vtk_model = vtkSmartPointer<vtkPolyData>::New ();
    vtk_model->DeepCopy (static_cast<vtkPolyData*> (it->user_data_));

    // Setup the matrix
    vtkSmartPointer<vtkMatrix4x4> vtk_mat = vtkSmartPointer<vtkMatrix4x4>::New ();
    vtk_mat->Identity ();
    const float *t = it->rigid_transform_;
    // Setup the rotation
    vtk_mat->SetElement (0, 0, t[0]); vtk_mat->SetElement (0, 1, t[1]); vtk_mat->SetElement (0, 2, t[2]);
    vtk_mat->SetElement (1, 0, t[3]); vtk_mat->SetElement (1, 1, t[4]); vtk_mat->SetElement (1, 2, t[5]);
    vtk_mat->SetElement (2, 0, t[6]); vtk_mat->SetElement (2, 1, t[7]); vtk_mat->SetElement (2, 2, t[8]);
    // Setup the translation
    vtk_mat->SetElement (0, 3, t[9]); vtk_mat->SetElement (1, 3, t[10]); vtk_mat->SetElement (2, 3, t[11]);

    // Setup the transform based on the matrix
    vtkSmartPointer<vtkTransform> vtk_transform = vtkSmartPointer<vtkTransform>::New ();
    vtk_transform->SetMatrix (vtk_mat);

    // Setup the transformator
    vtkSmartPointer<vtkTransformPolyDataFilter> vtk_transformator = vtkSmartPointer<vtkTransformPolyDataFilter>::New ();
    vtk_transformator->SetTransform (vtk_transform);
    vtk_transformator->SetInput (vtk_model);
    vtk_transformator->Update ();

    // Visualize
    vtkSmartPointer<vtkActor> vtk_actor = vtkSmartPointer<vtkActor>::New();
    vtkSmartPointer<vtkPolyDataMapper> vtk_mapper = vtkSmartPointer<vtkPolyDataMapper>::New ();
    vtk_mapper->SetInput(vtk_transformator->GetOutput ());
    vtk_actor->SetMapper(vtk_mapper);
    // Set the appearance & add to the renderer
    vtk_actor->GetProperty ()->SetColor (0.6, 0.7, 0.9);
    renderer->AddActor(vtk_actor);
    params->actors_.push_back (vtk_actor);

#ifdef _SHOW_TRANSFORM_SPACE_
    if ( transform_space.getPositionCellBounds ((*acc_hypo)->pos_id_, cb) )
    {
      sprintf (pos_cell_name, "cell [%i, %i, %i]\n", (*acc_hypo)->pos_id_[0], (*acc_hypo)->pos_id_[1], (*acc_hypo)->pos_id_[2]);
      params->viz_.addCube (cb[0], cb[1], cb[2], cb[3], cb[4], cb[5], 1.0, 1.0, 1.0, pos_cell_name);
    }
    else
      printf ("WARNING: no cell at position [%i, %i, %i]\n", (*acc_hypo)->pos_id_[0], (*acc_hypo)->pos_id_[1], (*acc_hypo)->pos_id_[2]);
#endif
  }
}

//===============================================================================================================================

bool
vtk2PointCloud (const char* file_name, PointCloud<PointXYZ>& pcl_points, PointCloud<Normal>& pcl_normals, vtkPolyData* vtk_data)
{
  size_t len = strlen (file_name);
  if ( file_name[len-3] != 'v' || file_name[len-2] != 't' || file_name[len-1] != 'k' )
  {
    fprintf (stderr, "ERROR: we need a .vtk object!\n");
    return false;
  }

  // Load the model
  vtkSmartPointer<vtkPolyDataReader> reader = vtkSmartPointer<vtkPolyDataReader>::New ();
  reader->SetFileName (file_name);
  reader->Update ();

  // Get the points
  vtkPolyData *vtk_poly = reader->GetOutput ();
  vtkPoints *vtk_points = vtk_poly->GetPoints ();
  vtkIdType num_points = vtk_points->GetNumberOfPoints ();
  double p[3];

  // Shall we copy the vtk object
  if ( vtk_data )
    vtk_data->DeepCopy (vtk_poly);

  pcl_points.resize (num_points);

  // Copy the points
  for ( vtkIdType i = 0 ; i < num_points ; ++i )
  {
    vtk_points->GetPoint (i, p);
    pcl_points[i].x = static_cast<float> (p[0]);
    pcl_points[i].y = static_cast<float> (p[1]);
    pcl_points[i].z = static_cast<float> (p[2]);
  }

  // Check if we have normals
  vtkDataArray *vtk_normals = vtk_poly->GetPointData ()->GetNormals ();
  if ( !vtk_normals )
    return false;

  pcl_normals.resize (num_points);
  // Copy the normals
  for ( vtkIdType i = 0 ; i < num_points ; ++i )
  {
    vtk_normals->GetTuple (i, p);
    pcl_normals[i].normal_x = static_cast<float> (p[0]);
    pcl_normals[i].normal_y = static_cast<float> (p[1]);
    pcl_normals[i].normal_z = static_cast<float> (p[2]);
  }

  return true;
}

//===============================================================================================================================