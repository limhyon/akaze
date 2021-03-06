//=============================================================================
//
// akaze_match.cpp
// Authors: Pablo F. Alcantarilla (1), Jesus Nuevo (2)
// Institutions: Georgia Institute of Technology (1)
//               TrueVision Solutions (2)
// Date: 15/09/2013
// Email: pablofdezalc@gmail.com
//
// AKAZE Features Copyright 2013, Pablo F. Alcantarilla, Jesus Nuevo
// All Rights Reserved
// See LICENSE for the license information
//=============================================================================

/**
 * @file akaze_match.cpp
 * @brief Main program for matching two images with AKAZE features
 * @date Sep 15, 2013
 * @author Pablo F. Alcantarilla
 *
 * Modification:
 * 16/11/2013: David Ok (david.ok8@gmail.com)
 *
 */

#include "akaze_compare.h"
#include "cmdLine.h"

using namespace std;
using namespace cv;

int main( int argc, char *argv[]) {

  // Variables
  AKAZEOptions options;
  Mat img1, img1_32, img2, img2_32;
  string img_name1, img_name2, hfile;
  string rfile;
  double t1 = 0.0, t2 = 0.0;

  // ORB variables
  Ptr<OrbFeatureDetector> orb_detector;
  Ptr<DescriptorExtractor> orb_descriptor;
  vector<KeyPoint> kpts1_orb, kpts2_orb;
  vector<Point2f> matches_orb, inliers_orb;
  vector<vector<DMatch> > dmatches_orb;
  Mat desc1_orb, desc2_orb;
  int nmatches_orb = 0, ninliers_orb = 0, noutliers_orb = 0;
  int nkpts1_orb = 0, nkpts2_orb = 0;
  float ratio_orb = 0.0;
  double torb = 0.0;

  // BRISK variables
  BRISK dbrisk(BRISK_HTHRES,BRISK_NOCTAVES);
  vector<KeyPoint> kpts1_brisk, kpts2_brisk;
  vector<Point2f> matches_brisk, inliers_brisk;
  vector<vector<DMatch> > dmatches_brisk;
  Mat desc1_brisk, desc2_brisk;
  int nmatches_brisk = 0, ninliers_brisk = 0, noutliers_brisk = 0;
  int nkpts1_brisk = 0, nkpts2_brisk = 0;
  float ratio_brisk = 0.0;
  double tbrisk = 0.0;

  // AKAZE variables
  vector<KeyPoint> kpts1_akaze, kpts2_akaze;
  vector<Point2f> matches_akaze, inliers_akaze;
  vector<vector<DMatch> > dmatches_akaze;
  Mat desc1_akaze, desc2_akaze;
  int nmatches_akaze = 0, ninliers_akaze = 0, noutliers_akaze = 0;
  int nkpts1_akaze = 0, nkpts2_akaze = 0;
  float ratio_akaze = 0.0;
  double takaze = 0.0;

  Ptr<cv::DescriptorMatcher> matcher_l2 = DescriptorMatcher::create("BruteForce");
  Ptr<cv::DescriptorMatcher> matcher_l1 = DescriptorMatcher::create("BruteForce-Hamming");
  Mat HG;

  // Parse the input command line options
  if (!parse_input_options(options,img_name1,img_name2,hfile,rfile,argc,argv)) {
    return -1;
  }

  // Read the image, force to be grey scale
  img1 = imread(img_name1,0);

  if (img1.data == NULL) {
    cout << "Error loading image: " << img_name1 << endl;
    return -1;
  }

  // Read the image, force to be grey scale
  img2 = imread(img_name2,0);

  if (img2.data == NULL) {
    cout << "Error loading image: " << img_name2 << endl;
    return -1;
  }

  // Convert the images to float
  img1.convertTo(img1_32,CV_32F,1.0/255.0,0);
  img2.convertTo(img2_32,CV_32F,1.0/255.0,0);

  // Color images for results visualization
  Mat img1_rgb_orb = Mat(Size(img1.cols,img1.rows),CV_8UC3);
  Mat img2_rgb_orb = Mat(Size(img2.cols,img1.rows),CV_8UC3);
  Mat img_com_orb = Mat(Size(img1.cols*2,img1.rows),CV_8UC3);

  Mat img1_rgb_brisk = Mat(Size(img1.cols,img1.rows),CV_8UC3);
  Mat img2_rgb_brisk = Mat(Size(img2.cols,img1.rows),CV_8UC3);
  Mat img_com_brisk = Mat(Size(img1.cols*2,img1.rows),CV_8UC3);

  Mat img1_rgb_akaze = Mat(Size(img1.cols,img1.rows),CV_8UC3);
  Mat img2_rgb_akaze = Mat(Size(img2.cols,img1.rows),CV_8UC3);
  Mat img_com_akaze = Mat(Size(img1.cols*2,img1.rows),CV_8UC3);

  // Read the homography file
  read_homography(hfile,HG);

  //*************************************************************************************
  //*************************************************************************************

  // ORB Features
  //*****************
  orb_detector = new cv::OrbFeatureDetector(ORB_MAX_KPTS,ORB_SCALE_FACTOR,ORB_PYRAMID_LEVELS,
      ORB_EDGE_THRESHOLD,ORB_FIRST_PYRAMID_LEVEL,ORB_WTA_K,ORB_PATCH_SIZE);
  orb_descriptor = new cv::OrbDescriptorExtractor();

  t1 = getTickCount();

  orb_detector->detect(img1,kpts1_orb);
  orb_detector->detect(img2,kpts2_orb);

  nkpts1_orb = kpts1_orb.size();
  nkpts2_orb = kpts2_orb.size();

  orb_descriptor->compute(img1,kpts1_orb,desc1_orb);
  orb_descriptor->compute(img2,kpts2_orb,desc2_orb);

  matcher_l1->knnMatch(desc1_orb,desc2_orb,dmatches_orb,2);

  matches2points_nndr(kpts1_orb,kpts2_orb,dmatches_orb,matches_orb,DRATIO);
  compute_inliers_homography(matches_orb,inliers_orb,HG,MIN_H_ERROR);

  nmatches_orb = matches_orb.size()/2;
  ninliers_orb = inliers_orb.size()/2;
  noutliers_orb = nmatches_orb-ninliers_orb;
  ratio_orb = 100.0*(float)(ninliers_orb)/(float)(nmatches_orb);

  t2 = getTickCount();
  torb = 1000.0*(t2-t1) / getTickFrequency();

  cvtColor(img1,img1_rgb_orb,CV_GRAY2BGR);
  cvtColor(img2,img2_rgb_orb,CV_GRAY2BGR);

  draw_keypoints(img1_rgb_orb,kpts1_orb);
  draw_keypoints(img2_rgb_orb,kpts2_orb);
  draw_inliers(img1_rgb_orb,img2_rgb_orb,img_com_orb,inliers_orb,0);

  cout << "ORB Results" << endl;
  cout << "**************************************" << endl;
  cout << "Number of Keypoints Image 1: " << nkpts1_orb << endl;
  cout << "Number of Keypoints Image 2: " << nkpts2_orb << endl;
  cout << "Number of Matches: " << nmatches_orb << endl;
  cout << "Number of Inliers: " << ninliers_orb << endl;
  cout << "Number of Outliers: " << noutliers_orb << endl;
  cout << "Inliers Ratio: " << ratio_orb << endl;
  cout << "ORB Features Extraction Time (ms): " << torb << endl;
  cout << endl;

  //*************************************************************************************
  //*************************************************************************************

  // BRISK Features
  //*****************
  t1 = getTickCount();

  dbrisk(img1,noArray(),kpts1_brisk,desc1_brisk,false);
  dbrisk(img2,noArray(),kpts2_brisk,desc2_brisk,false);

  matcher_l1->knnMatch(desc1_brisk,desc2_brisk,dmatches_brisk,2);

  matches2points_nndr(kpts1_brisk,kpts2_brisk,dmatches_brisk,matches_brisk,DRATIO);
  compute_inliers_homography(matches_brisk,inliers_brisk,HG,MIN_H_ERROR);

  nkpts1_brisk = kpts1_brisk.size();
  nkpts2_brisk= kpts2_brisk.size();
  nmatches_brisk = matches_brisk.size()/2;
  ninliers_brisk = inliers_brisk.size()/2;
  noutliers_brisk = nmatches_brisk-ninliers_brisk;
  ratio_brisk = 100.0*(float)(ninliers_brisk)/(float)(nmatches_brisk);

  t2 = getTickCount();
  tbrisk = 1000.0*(t2-t1) / getTickFrequency();

  cvtColor(img1,img1_rgb_brisk,CV_GRAY2BGR);
  cvtColor(img2,img2_rgb_brisk,CV_GRAY2BGR);

  draw_keypoints(img1_rgb_brisk,kpts1_brisk);
  draw_keypoints(img2_rgb_brisk,kpts2_brisk);
  draw_inliers(img1_rgb_brisk,img2_rgb_brisk,img_com_brisk,inliers_brisk,1);

  cout << "BRISK Results" << endl;
  cout << "**************************************" << endl;
  cout << "Number of Keypoints Image 1: " << nkpts1_brisk << endl;
  cout << "Number of Keypoints Image 2: " << nkpts2_brisk << endl;
  cout << "Number of Matches: " << nmatches_brisk << endl;
  cout << "Number of Inliers: " << ninliers_brisk << endl;
  cout << "Number of Outliers: " << noutliers_brisk << endl;
  cout << "Inliers Ratio: " << ratio_brisk << endl;
  cout << "BRISK Features Extraction Time (ms): " << tbrisk << endl;
  cout << endl;

  //*************************************************************************************
  //*************************************************************************************

  // A-KAZE Features
  //*******************
  options.img_width = img1.cols;
  options.img_height = img1.rows;
  AKAZE evolution1(options);

  options.img_width = img2.cols;
  options.img_height = img2.rows;
  AKAZE evolution2(options);

  t1 = getTickCount();

  evolution1.Create_Nonlinear_Scale_Space(img1_32);
  evolution1.Feature_Detection(kpts1_akaze);
  evolution1.Compute_Descriptors(kpts1_akaze,desc1_akaze);

  evolution2.Create_Nonlinear_Scale_Space(img2_32);
  evolution2.Feature_Detection(kpts2_akaze);
  evolution2.Compute_Descriptors(kpts2_akaze,desc2_akaze);

  nkpts1_akaze = kpts1_akaze.size();
  nkpts2_akaze = kpts2_akaze.size();

  if (options.descriptor < MLDB_UPRIGHT) {
    matcher_l2->knnMatch(desc1_akaze,desc2_akaze,dmatches_akaze,2);
  }
  // Binary descriptor, use Hamming distance
  else {
    matcher_l1->knnMatch(desc1_akaze,desc2_akaze,dmatches_akaze,2);
  }

  matches2points_nndr(kpts1_akaze,kpts2_akaze,dmatches_akaze,matches_akaze,DRATIO);
  compute_inliers_homography(matches_akaze,inliers_akaze,HG,MIN_H_ERROR);

  t2 = getTickCount();
  takaze = 1000.0*(t2-t1)/getTickFrequency();

  nmatches_akaze = matches_akaze.size()/2;
  ninliers_akaze = inliers_akaze.size()/2;
  noutliers_akaze = nmatches_akaze-ninliers_akaze;
  ratio_akaze = 100.0*((float) ninliers_akaze / (float) nmatches_akaze);

  cvtColor(img1,img1_rgb_akaze,CV_GRAY2BGR);
  cvtColor(img2,img2_rgb_akaze,CV_GRAY2BGR);

  draw_keypoints(img1_rgb_akaze,kpts1_akaze);
  draw_keypoints(img2_rgb_akaze,kpts2_akaze);
  draw_inliers(img1_rgb_akaze,img2_rgb_akaze,img_com_akaze,inliers_akaze,2);

  cout << "A-KAZE Results" << endl;
  cout << "**************************************" << endl;
  cout << "Number of Keypoints Image 1: " << nkpts1_akaze << endl;
  cout << "Number of Keypoints Image 2: " << nkpts2_akaze << endl;
  cout << "Number of Matches: " << nmatches_akaze << endl;
  cout << "Number of Inliers: " << ninliers_akaze << endl;
  cout << "Number of Outliers: " << noutliers_akaze << endl;
  cout << "Inliers Ratio: " << ratio_akaze << endl;
  cout << "A-KAZE Features Extraction Time (ms): " << takaze << endl;
  cout << endl;

  // Show the images with the inliers
  imshow("ORB",img_com_orb);
  imshow("BRISK",img_com_brisk);
  imshow("A-KAZE",img_com_akaze);
  waitKey(0);
}

bool parse_input_options(AKAZEOptions &options,
                        std::string& img_name1, std::string& img_name2,
                        std::string& hom, std::string& kfile,
                        int argc, char *argv[])
{
  // Create command line options.
  CmdLine cmdLine;
  cmdLine.add(make_switch('h', "help"));
  // Verbose option for debug.
  cmdLine.add(make_option('v', options.verbosity, "verbose"));
  // Image file name.
  cmdLine.add(make_option('L', img_name1, "left image, i.e. path of image 1"));
  cmdLine.add(make_option('R', img_name2, "right image, i.e. path of image 2"));
  cmdLine.add(make_option('H', hom, "ground truth homography"));
  // Scale-space parameters.
  cmdLine.add(make_option('O', options.omax, "omax"));
  cmdLine.add(make_option('S', options.nsublevels, "nsublevels"));
  cmdLine.add(make_option('s', options.soffset, "soffset"));
  cmdLine.add(make_option('d', options.sderivatives, "sderivatives"));
  cmdLine.add(make_option('g', options.diffusivity, "diffusivity"));
  // Detection parameters.
  cmdLine.add(make_option('a', options.dthreshold, "dthreshold"));
  cmdLine.add(make_option('b', options.dthreshold2, "dthreshold2")); // ?????
  // Descriptor parameters.
  cmdLine.add(make_option('D', options.descriptor, "descriptor"));
  cmdLine.add(make_option('C', options.descriptor_channels, "descriptor_channels"));
  cmdLine.add(make_option('F', options.descriptor_size, "descriptor_size"));
  // Save the keypoints.
  cmdLine.add(make_option('o', kfile, "output"));
  // Save scale-space
  cmdLine.add(make_option('w', options.save_scale_space, "save_scale_space"));

  // Try to process
  try
  {
    if (argc == 1)
      throw std::string("Invalid command line parameter.");

    cmdLine.process(argc, argv);

    if (!cmdLine.used('L') || !cmdLine.used('R') || !cmdLine.used('H'))
      throw std::string("Invalid command line parameter.");
  }
  catch(const std::string& s)
  {
    show_input_options_help(2);
    return false;
  }

  return true;
}