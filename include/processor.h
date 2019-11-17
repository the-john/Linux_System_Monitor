#ifndef PROCESSOR_H
#define PROCESSOR_H

class Processor {
 public:
  float Utilization();  // TODO: See src/processor.cpp

  // TODO: Declare any necessary private members
 private:
   long totalJiffyStart = 0;
   long activeJiffyStart = 0;
   long totalJiffyStop = 0;
   long activeJiffyStop = 0;
   //long totalTime = 0;
   //long activeTime = 0;
};

#endif