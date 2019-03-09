#include <algorithm>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>
#include <ctime>
#include <stdio.h>
#include <unistd.h>
#if USE_MPI
#include <mpi.h>
#endif
#include "lulesh.h"

// --- Caliper helper functions

template<typename T>
void setGlobal(const char* name, T val){
   cali::Annotation(name, CALI_ATTR_GLOBAL | CALI_ATTR_SKIP_EVENTS).set(val);
}

// defined in lulesh-build-metadata.cc, which is generated by cmake
extern const char* buildMetadata[][2]; 

void RecordCaliperMetadata(const struct cmdLineOpts& opts)
{
   /** Not recording time, Caliper has issues with ':' in values? */
   //time(&timer);
   //struct tm* rt = localtime(&timer);

   std::string hostname = getenv("HOSTNAME");
   std::string cluster(hostname, 0, hostname.find_first_of("0123456789"));
  
   setGlobal("Cluster",           cluster.c_str());  
     
   setGlobal("Iterations",        opts.its);
   setGlobal("Problem Size",      opts.nx);
   setGlobal("Number of Regions", opts.numReg);
   setGlobal("Region Balance",    opts.balance);
   setGlobal("Region Cost",       opts.cost);

   // add build metadata
   for (size_t i = 0; buildMetadata[i][0]; ++i)
       setGlobal(buildMetadata[i][0], buildMetadata[i][1]);
  
   // setGlobal("Hostname", hostname.c_str()); // not global in MPI
   // setGlobal("Run Start Time",asctime(rt));
}

void EnableSpot()
{
   std::map<std::string, std::string> cfg = {
      { "CALI_SERVICES_ENABLE",          "event,aggregate,timestamp" },
      { "CALI_TIMER_INCLUSIVE_DURATION", "true" }
   };

   char   timestr[16];
   time_t tm = time(NULL);
   strftime(timestr, sizeof(timestr), "%y%m%d-%H%M%S", localtime(&tm));

   std::string filename =
       std::string(timestr) + "_lulesh_%Cluster%_" + std::to_string(getpid()) + ".cali";

#if USE_MPI
   cfg["CALI_SERVICES_ENABLE"      ].append(",mpi,mpireport");
   cfg["CALI_MPIREPORT_CONFIG"     ] =
      "select loop,function,mpi.function,min(sum#time.inclusive.duration),avg(sum#time.inclusive.duration),max(sum#time.inclusive.duration) group by prop:nested format cali";
   cfg["CALI_MPIREPORT_FILENAME"   ] = filename;
   cfg["CALI_CHANNEL_FLUSH_ON_EXIT"] = "false"; // mpireport flushes at MPI_Finalize
#else
   cfg["CALI_SERVICES_ENABLE"      ].append(",recorder");
   cfg["CALI_RECORDER_FILENAME"    ] = filename;
#endif
   
   cali::create_channel("spot", 0, cfg);
}


/* Helper function for converting strings to ints, with error checking */
int StrToInt(const char *token, int *retVal)
{
   const char *c ;
   char *endptr ;
   const int decimal_base = 10 ;

   if (token == NULL)
      return 0 ;
   
   c = token ;
   *retVal = (int)strtol(c, &endptr, decimal_base) ;
   if((endptr != c) && ((*endptr == ' ') || (*endptr == '\0')))
      return 1 ;
   else
      return 0 ;
}

static void PrintCommandLineOptions(char *execname, int myRank)
{
   if (myRank == 0) {

      printf("Usage: %s [opts]\n", execname);
      printf(" where [opts] is one or more of:\n");
      printf(" -q              : quiet mode - suppress all stdout\n");
      printf(" -i <iterations> : number of cycles to run\n");
      printf(" -s <size>       : length of cube mesh along side\n");
      printf(" -r <numregions> : Number of distinct regions (def: 11)\n");
      printf(" -b <balance>    : Load balance between regions of a domain (def: 1)\n");
      printf(" -c <cost>       : Extra cost of more expensive regions (def: 1)\n");
      printf(" -f <numfiles>   : Number of files to split viz dump into (def: (np+10)/9)\n");
      printf(" -p              : Print out progress\n");
      printf(" -g              : Produce Spot Caliper file\n");
      printf(" -v              : Output viz file (requires compiling with -DVIZ_MESH\n");
      printf(" -h              : This message\n");
      printf("\n\n");
   }
}

static void ParseError(const char *message, int myRank)
{
   if (myRank == 0) {
      printf("%s\n", message);
#if USE_MPI      
      MPI_Abort(MPI_COMM_WORLD, -1);
#else
      exit(-1);
#endif
   }
}

void ParseCommandLineOptions(int argc, char *argv[],
                             int myRank, struct cmdLineOpts *opts)
{
   if(argc > 1) {
      int i = 1;

      while(i < argc) {
         int ok;
         /* -i <iterations> */
         if(strcmp(argv[i], "-i") == 0) {
            if (i+1 >= argc) {
               ParseError("Missing integer argument to -i", myRank);
            }
            ok = StrToInt(argv[i+1], &(opts->its));
            if(!ok) {
               ParseError("Parse Error on option -i integer value required after argument\n", myRank);
            }
            i+=2;
         }
         /* -s <size, sidelength> */
         else if(strcmp(argv[i], "-s") == 0) {
            if (i+1 >= argc) {
               ParseError("Missing integer argument to -s\n", myRank);
            }
            ok = StrToInt(argv[i+1], &(opts->nx));
            if(!ok) {
               ParseError("Parse Error on option -s integer value required after argument\n", myRank);
            }
            i+=2;
         }
	 /* -r <numregions> */
         else if (strcmp(argv[i], "-r") == 0) {
            if (i+1 >= argc) {
               ParseError("Missing integer argument to -r\n", myRank);
            }
            ok = StrToInt(argv[i+1], &(opts->numReg));
            if (!ok) {
               ParseError("Parse Error on option -r integer value required after argument\n", myRank);
            }
            i+=2;
         }
	 /* -f <numfilepieces> */
         else if (strcmp(argv[i], "-f") == 0) {
            if (i+1 >= argc) {
               ParseError("Missing integer argument to -f\n", myRank);
            }
            ok = StrToInt(argv[i+1], &(opts->numFiles));
            if (!ok) {
               ParseError("Parse Error on option -f integer value required after argument\n", myRank);
            }
            i+=2;
         }
         /* -p */
         else if (strcmp(argv[i], "-p") == 0) {
            opts->showProg = 1;
            i++;
         }
         /* -q */
         else if (strcmp(argv[i], "-q") == 0) {
            opts->quiet = 1;
            i++;
         }
         else if (strcmp(argv[i], "-g") == 0) {
            opts->spot = 1;
            i++;
         }
         else if (strcmp(argv[i], "-b") == 0) {
            if (i+1 >= argc) {
               ParseError("Missing integer argument to -b\n", myRank);
            }
            ok = StrToInt(argv[i+1], &(opts->balance));
            if (!ok) {
               ParseError("Parse Error on option -b integer value required after argument\n", myRank);
            }
            i+=2;
         }
         else if (strcmp(argv[i], "-c") == 0) {
            if (i+1 >= argc) {
               ParseError("Missing integer argument to -c\n", myRank);
            }
            ok = StrToInt(argv[i+1], &(opts->cost));
            if (!ok) {
               ParseError("Parse Error on option -c integer value required after argument\n", myRank);
            }
            i+=2;
         }
         /* -v */
         else if (strcmp(argv[i], "-v") == 0) {
#if VIZ_MESH            
            opts->viz = 1;
#else
            ParseError("Use of -v requires compiling with -DVIZ_MESH\n", myRank);
#endif
            i++;
         }
         /* -h */
         else if (strcmp(argv[i], "-h") == 0) {
            PrintCommandLineOptions(argv[0], myRank);
#if USE_MPI            
            MPI_Abort(MPI_COMM_WORLD, 0);
#else
            exit(0);
#endif
         }
         else {
            char msg[80];
            PrintCommandLineOptions(argv[0], myRank);
            sprintf(msg, "ERROR: Unknown command line argument: %s\n", argv[i]);
            ParseError(msg, myRank);
         }
      }
   }
}

/////////////////////////////////////////////////////////////////////

void VerifyAndWriteFinalOutput(Real_t elapsed_time,
                               Domain& locDom,
                               Int_t nx,
                               Int_t numRanks)
{
   // GrindTime1 only takes a single domain into account, and is thus a good way to measure
   // processor speed indepdendent of MPI parallelism.
   // GrindTime2 takes into account speedups from MPI parallelism 
   Real_t grindTime1 = ((elapsed_time*1e6)/locDom.cycle())/(nx*nx*nx);
   Real_t grindTime2 = ((elapsed_time*1e6)/locDom.cycle())/(nx*nx*nx*numRanks);

   Index_t ElemId = 0;
   printf("Run completed:  \n");
   printf("   Problem size        =  %i \n",    nx);
   printf("   MPI tasks           =  %i \n",    numRanks);
   printf("   Iteration count     =  %i \n",    locDom.cycle());
   printf("   Final Origin Energy = %12.6e \n", locDom.e(ElemId));

   Real_t   MaxAbsDiff = Real_t(0.0);
   Real_t TotalAbsDiff = Real_t(0.0);
   Real_t   MaxRelDiff = Real_t(0.0);

   for (Index_t j=0; j<nx; ++j) {
      for (Index_t k=j+1; k<nx; ++k) {
         Real_t AbsDiff = FABS(locDom.e(j*nx+k)-locDom.e(k*nx+j));
         TotalAbsDiff  += AbsDiff;

         if (MaxAbsDiff <AbsDiff) MaxAbsDiff = AbsDiff;

         Real_t RelDiff = AbsDiff / locDom.e(k*nx+j);

         if (MaxRelDiff <RelDiff)  MaxRelDiff = RelDiff;
      }
   }

   // Quick symmetry check
   printf("   Testing Plane 0 of Energy Array on rank 0:\n");
   printf("        MaxAbsDiff   = %12.6e\n",   MaxAbsDiff   );
   printf("        TotalAbsDiff = %12.6e\n",   TotalAbsDiff );
   printf("        MaxRelDiff   = %12.6e\n\n", MaxRelDiff   );

   // Timing information
   printf("\nElapsed time         = %10.2f (s)\n", elapsed_time);
   printf("Grind time (us/z/c)  = %10.8g (per dom)  (%10.8g overall)\n", grindTime1, grindTime2);
   printf("FOM                  = %10.8g (z/s)\n\n", 1000.0/grindTime2); // zones per second
   
   setGlobal("FigureOfMerit",1000.0/grindTime2);

   return ;
}

