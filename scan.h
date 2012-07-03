#ifndef SCAN
#define SCAN

#include <sstream>
#include "../scan/data4scan.h"
#include "../stepper/stepper.h"
//#include "../stepper/stepper_factory.h"
#include "../grab/grab_factory.h"

//! make a full scanning
/**
 * scanning types:
 * - scanning : scan but no check (especially use full for all ready recorded scan)
 * - scanning_force : scan with position reader, so it could try several times to make the position ordered one after the other (might be long; internal try loop)
 * - scanning_lazy  : scan with position reader, make a displacement and record it if not done yet (might be faster; external try loop)
 * make displacements and record images.
 * note: embendding GUI for progress
**/
class Cscan
{
public:
  //! class name for debug only
#if cimg_debug>1
  std::string class_name;
#endif
  //! class (or library) version for information only
  std::string class_version;
private:
  //! grab device

  //! stepper device

public:
  //! constructor
  /**
   *
  **/
  Cscan()
  {
#if cimg_debug>1
    class_name="Cscan";
#endif
    class_version=SCAN_VERSION;
  }//constructor

//attributes:
//grab
//stepper
//data

//functions:
///image_file_name
///record_images
///scanning
////scanning_force
////scanning_lazy

//! set numbered image file name from file path format
/**
 * 
 * \param [out] file_name: image file name with numbers from i,j,k,l (e.g. ./img_x00_y00_z00_i000.png)
 * \param [in] file_path_format: format for the image file name (e.g. ./img_x%02d_y%02d_z%02d_i%03d.png)
 * \param [in] i,j,k,l: dimension index for current file name (e.g. 0,0,0,0)
**/
bool image_file_name(std::string &file_name,const std::string &file_path_format, int i,int j,int k,int l)
{
  char fileAsCA[512];
  std::sprintf((char*)fileAsCA/*.c_str()*/,file_path_format.c_str(),i,j,k,l);//e.g. file_path_format="./img_x%02d_y%02d_z%02d_i%03d.png"
  file_name=fileAsCA;
  return true;
}//image_file_name


//! grab a set of images and save them both to image file and its statistics to memory.
/**
 * 
 * \param [in] grab: camera device.
 * \param [out] image: last acquired image.
 * \param [in] ImagePath: format for the image file name (e.g. ./img_x%02d_y%02d_z%02d_i%03d.png)
 * \param [in] ImageNumber: number of images to record (e.g. size of set)
 * \param [in] i,j,k: dimension index for current image i.e. file name (e.g. 0,0,0)
 * \param [out] data4scan: statistics of recorded images and status map.
**/
int record_images(Cgrab &grab,cimg_library::CImg<int> &image,const std::string &ImagePath,const int ImageNumber,const int i,const int j,const int k,Cdata4scan<float,int> &data4scan)
{
//std::cerr<<__FILE__<<"/"<<__func__<<": grab type="<<grab.class_name<<".\n"<<std::flush;
  std::string file;
  for(int l=0;l<ImageNumber;++l)
  {//do
    image_file_name(file,ImagePath,i,j,k,l);
std::cerr<<"file=\""<<file<<"\"\n"<<std::flush;
    if(!grab.grab(image,file)) return 1;
    if(l==0&&i==0&&j==0&&k==0)
    {//set first full image information
      data4scan.set_first_full_image_information(image);
    }//first full image
std::cerr<<"warning: no crop (in "<<__FILE__<<"/"<<__func__<<"function )\n"<<std::flush;//! \todo set crop value from data4scan width and height (i.e. make a crop_sample function AND call it here)
    data4scan.add_sample(image,i,j,k);
 }//done      end of grab images
  //compute mean image
//! \todo [low] set data4scan type (factory) or add maximum and minimum variable within it
  data4scan.normalise(i,j,k);
  //set flag
  data4scan.flag(i,j,k)=1;//satisfied
  return 0;
}//record_images

//! scan a full volume, i.e. all positions of the volume (note: reset to original position with mechanical jitter)
/**
 * scan a volume, i.e. make all displacement along the 3 axes to do each position in the volume.
 * \note reset to original position, especially in order to make 3D position scanning in each direction loop.
 * \note could be used as 1D line, column or depth scanning for any axis scan, or even 2D plane or slice scanning.
 *
 * \param[in] stepper: displacement stage device (should be already open)
 * \param[in] number: number of iteration in each of the scanning loops (i.e. number of positions for the 3 axes; size should be 3)
 * \param[in] step: 3D displacement in step (i.e. size should be 3)
 * \param[in] velocity: 3D velocity in step per second (i.e. size should be 3)
 * \param[in] wait_time: minimum delay between each loop displacement
 * \param[in] mechanical_jitter: mechanical jitter to perform a good reset for any axes
**/
int scanning(Cstepper &stepper,const cimg_library::CImg<int> &number,const cimg_library::CImg<int> &step,const cimg_library::CImg<int> &velocity,const int wait_time, const unsigned int mechanical_jitter
#if cimg_display>0
,const unsigned int zoom=100,const bool do_display=false
#endif //cimg_display
)
{
///set signed mechanical jitter
  //set mechanical jitter for all axes with same sign as corresponding displacement.
  cimg_library::CImg<int>  mj(3);
  cimg_forX(mj,a) mj(a)=((step(a)>0)?1:-1)*mechanical_jitter;

///reset to (0,0,0): NOT for this function

///scanning message
  std::cerr<<"full scanning volume of "<<
 #ifdef cimg_use_vt100
  cimg_library::cimg::t_purple<<
 #endif
  "size (sX,sY,sZ)=("<<number(0)*step(0)<<","<<number(1)*step(1)<<","<<number(2)*step(2)<<") steps"<<
 #ifdef cimg_use_vt100
  cimg_library::cimg::t_normal<<
 #endif
  " by "<<
 #ifdef cimg_use_vt100
  cimg_library::cimg::t_purple<<
 #endif
  "displacement (dX,dY,dZ)=("<<step(0)<<","<<step(1)<<","<<step(2)<<") steps"<<
 #ifdef cimg_use_vt100
  cimg_library::cimg::t_normal<<
 #endif
  " at (vX,vY,vZ)=("<<velocity(0)<<","<<velocity(1)<<","<<velocity(2)<<") step(s) per second speed.\n"<<std::flush;

#if cimg_display>0
  //GUI to display scanning progress
  cimg_library::CImg<char> volume(number(0),number(1),number(2));volume.fill(2);//0=fail(red), 1=done(green), 2=to_do(blue)
  //color
//! \todo use \c volume for setting colors to \c colume (e.g. \c red color in case of position failure; need position check stepper)
  const unsigned char red[] = { 255,0,0 }, green[] = { 0,255,0 }, blue[] = { 0,0,255 };
  cimg_library::CImg<unsigned char> colume;
  //display
  cimg_library::CImgDisplay progress;
  if(do_display)
  {
//! \todo assign both \c colume and \c progress for displaying at best an image (i.e. 2D)
#if version_cimg < 130
    colume.assign(volume.width  ,volume.height,1,3);
    progress.assign(volume.width  *zoom,volume.height  *zoom);//,volume.depth()*zoom);
#else
    colume.assign(volume.width(),volume.height(),1,3);
    progress.assign(volume.width()*zoom,volume.height()*zoom);//,volume.depth()*zoom);
#endif
    progress.set_title("scan progress");
  }
#endif //cimg_display

///* Z axis loop
  //set 1D displacement for Z axis
  cimg_library::CImg<int> stepz(3);stepz.fill(0);stepz(2)=step(2);//e.g. (0,0,10)
  //Z axis loop
  for(int k=0;k<number(2);++k)
  {

#if cimg_display>0
    if(do_display)
    {
    //current slice
      cimg_forXY(colume,x,y) colume.draw_point(x,y,blue);
//! \todo remove slice in title if number(2)==1
      progress.set_title("#%d/%d: scan progress (slice#%d/%d)",k,number(2),k,number(2));
    }//do_display
#endif //cimg_display

///** Y axis loop
    //set 1D displacement for Y axis
    cimg_library::CImg<int> stepy(3);stepy.fill(0);stepy(1)=step(1);//e.g. (0,10,0)
    //Y axis loop
    for(int j=0;j<number(1);++j)
    {
///*** X axis loop
      //set 1D displacement for X axis
      cimg_library::CImg<int> stepx(3);stepx.fill(0);stepx(0)=step(0);//e.g. (10,0,0)
      //X axis loop
      for(int i=0;i<number(0);++i)
      {
///**** position message
        std::cerr << "actual displacement to "<<
       #ifdef cimg_use_vt100
        cimg_library::cimg::t_green<<
       #endif
        "(X,Y,Z)=("<<i*step(0)<<","<<j*step(1)<<","<<k*step(2)<<") "<<
       #ifdef cimg_use_vt100
        cimg_library::cimg::t_normal<<
       #endif
        "step position over entire scanning of ("<<number(0)*step(0)<<","<<number(1)*step(1)<<","<<number(2)*step(2)<<") steps.\n"<<std::flush;
///**** grab
//! \todo [high] add image grabbing.
//        record_images(...);
#if cimg_display>0
        //set status
        volume(i,j,k)=1;
        if(do_display)
        {
          //GUI to display scanning progress
          colume.draw_point(i,j,green);
          progress.display(colume.get_resize(zoom));
        }//do_display
#endif //cimg_display
///**** move along X axis
        //move along X axis
        if(number(0)>1)
        {//X move only if more than one line to do
          if(!stepper.move(stepx,velocity)) return 1;
        }//X move
///**** wait a while for user
        cimg_library::cimg::wait(wait_time);
      }//X axis loop
///*** reset X axis
      //go back to zero on X axis (i.e. move backward along X axis)
      if(number(0)>1)
      {//X reset (with mechanical jitter)
        // 0. reset message
        std::cerr<<
       #ifdef cimg_use_vt100
        cimg_library::cimg::t_purple<<
       #endif
        "reset X axis to (X,Y,Z)=(0,"<<j*step(1)<<","<<k*step(2)<<")"<<
       #ifdef cimg_use_vt100
        cimg_library::cimg::t_normal<<
       #endif
        ".\n"<<std::flush;
        // 1. move backward with mechanical jitter in X step // mechanical jitter = mj
        stepx(0)=-(step(0)*number(0)+mj(0));
        if(!stepper.move(stepx,velocity)) return 1;
        cimg_library::cimg::wait(wait_time);
        // 2. move forward mechanical jitter in X step
        stepx(0)=mj(0);
        if(!stepper.move(stepx,velocity)) return 1;
        cimg_library::cimg::wait(wait_time);
      }//X reset
///*** move along Y axis
      //move along Y axis
      if(number(1)>1)
      {//Y move only if more than one column to do
        if(!stepper.move(stepy,velocity)) return 1;
      }//Y move
///*** wait a while for user
      cimg_library::cimg::wait(wait_time);
    }//Y axis loop
///** reset Y axis
    //go back to zero on Y axis (i.e. move backward along Y axis)
    if(number(1)>1)
    {//Y reset (with mechanical jitter)
      // 0. reset message
      std::cerr<<
     #ifdef cimg_use_vt100
      cimg_library::cimg::t_purple<<
     #endif
      "reset Y axis to (X,Y,Z)=(0,0,"<<k*step(2)<<")"<<
     #ifdef cimg_use_vt100
      cimg_library::cimg::t_normal<<
     #endif
      ".\n"<<std::flush;
      // 1. move backward with mechanical jitter in Y step // mechanical jitter = mj
      stepy(1)=-(step(1)*number(1)+mj(1));
      if(!stepper.move(stepy,velocity)) return 1;
      cimg_library::cimg::wait(wait_time);
      // 2. move forward mechanical jitter in Y step
      stepy(1)=mj(1);
      if(!stepper.move(stepy,velocity)) return 1;
      cimg_library::cimg::wait(wait_time);
    }//Y reset
///** move along Z axis
    //move along Z axis
    if(number(2)>1)
    {//Z move only if more than one slice to do
      if(!stepper.move(stepz,velocity)) return 1;
    }//Z move
///** wait a while for user
    cimg_library::cimg::wait(wait_time);
  }//Z axis loop
///* reset Z axis
  //go back to zero on Z axis (i.e. move backward along Z axis)
  if(number(2)>1)
  {//Z reset (with mechanical jitter)
    // 0. reset message
    std::cerr<<
   #ifdef cimg_use_vt100
    cimg_library::cimg::t_purple<<
   #endif
    "reset Z axis to (X,Y,Z)=(0,0,0)"<<
   #ifdef cimg_use_vt100
    cimg_library::cimg::t_normal<<
   #endif
    ".\n"<<std::flush;
    // 1. move backward with mechanical jitter in Z step // mechanical jitter = mj
    stepz(2)=-(step(2)*number(2)+mj(2));
    if(!stepper.move(stepz,velocity)) return 1;
    cimg_library::cimg::wait(wait_time);
    // 2. move forward mechanical jitter in Z step
    stepz(2)=mj(2);
    if(!stepper.move(stepz,velocity)) return 1;
    cimg_library::cimg::wait(wait_time);
  }//Z reset

#if cimg_display>0
  //close GUI
  progress.close();
#endif //cimg_display

  return 0;
}//scanning

};//Cscan class

#endif //SCAN

