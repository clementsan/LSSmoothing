#include <iostream>

#include <itkImage.h>
#include <itkImageFileReader.h> 
#include <itkImageFileWriter.h>
#include <itkCastImageFilter.h>
#include <itkRescaleIntensityImageFilter.h>
#include <itkAntiAliasBinaryImageFilter.h>
#include <itkThresholdImageFilter.h>
#include <itkBinaryThresholdImageFilter.h>


using namespace itk;
using namespace std;

const int Dimension = 3;
typedef unsigned short ImagePixelType;
typedef float SmoothImagePixelType;
typedef Image<ImagePixelType,Dimension>  ImageType;
typedef Image<SmoothImagePixelType,Dimension> SmoothImageType;

typedef ImageFileReader<ImageType> VolumeReaderType;
typedef ImageFileWriter<ImageType> VolumeWriterType;
typedef ThresholdImageFilter< ImageType > threshFilterType;
typedef BinaryThresholdImageFilter< ImageType, ImageType > binThreshFilterType;
typedef CastImageFilter<ImageType ,SmoothImageType > CastToRealFilterType;
typedef AntiAliasBinaryImageFilter<SmoothImageType, SmoothImageType> AntiAliasFilterType;
typedef RescaleIntensityImageFilter<SmoothImageType, ImageType > RescaleFilter;
  
int main(int argc, const char* argv[])
{
  double maximumRMSError = 0.01;
  unsigned int numberOfIterations = 50;
  const int LABEL_VAL = 255;
  
  ImageType::Pointer inputImage;
  VolumeReaderType::Pointer labelReader = VolumeReaderType::New();

  // Reading input Image
  VolumeReaderType::Pointer imageReader = VolumeReaderType::New();
  imageReader->SetFileName(argv[1]) ;
  try
    {
      imageReader->Update() ;
    }
  catch (ExceptionObject err)
    {
      cerr<<"Exception object caught!"<<std::endl;
      cerr<<err<<std::endl;
      exit(0) ;
    }
  inputImage = imageReader->GetOutput();

  
  threshFilterType::Pointer threshFilter = threshFilterType::New();
  threshFilter->SetInput(inputImage);
  threshFilter->ThresholdAbove(0);
  threshFilter->SetOutsideValue (LABEL_VAL);
  threshFilter->Update();
  ImageType::Pointer procImage ;
  procImage = threshFilter->GetOutput();

  CastToRealFilterType::Pointer toReal = CastToRealFilterType::New();
  toReal->SetInput(procImage );
  
  AntiAliasFilterType::Pointer antiAliasFilter = AntiAliasFilterType::New();
  antiAliasFilter->SetInput( toReal->GetOutput() );
  antiAliasFilter->SetMaximumRMSError( maximumRMSError );
  antiAliasFilter->SetNumberOfIterations( numberOfIterations );
  antiAliasFilter->SetNumberOfLayers( 2 );
  antiAliasFilter->Update();
  
  RescaleFilter::Pointer rescale = RescaleFilter::New();
  rescale->SetInput( antiAliasFilter->GetOutput() );
  rescale->SetOutputMinimum(   0 );
  rescale->SetOutputMaximum( 255 );
  
  binThreshFilterType::Pointer binTreshFilter = binThreshFilterType::New();
  binTreshFilter->SetInput(rescale->GetOutput());
  binTreshFilter->SetUpperThreshold(255);
  binTreshFilter->SetLowerThreshold(127); 
  binTreshFilter->SetOutsideValue (0);
  binTreshFilter->SetInsideValue (1);
  binTreshFilter->Update();
  procImage = binTreshFilter->GetOutput();
  
  
  VolumeWriterType::Pointer writer = VolumeWriterType::New();
  writer->SetFileName(argv[2]);
  writer->SetInput(procImage);
  try
    {
      writer->Update();
    }
  catch (itk::ExceptionObject & err)
    {
      std::cerr<<"Exception object caught!"<<std::endl;
      std::cerr<<err<<std::endl;
    }
  return EXIT_SUCCESS;
}
