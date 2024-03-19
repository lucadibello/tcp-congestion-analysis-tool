// For parsing command line arguments
#include <getopt.h>

// Basic C libraries
#include <stdlib.h>
#include <stdio.h>
#include <string.h>  // For strcmp
#include <stdbool.h> // For bool type

// Socket programming libraries
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <arpa/inet.h>
#include <unistd.h> // for close
#include <netdb.h>  // for addrinfo

// For randomness
#include <time.h>

// For validating hostname
#include <regex.h>

// Default values for command line arguments
#define DEFAULT_PORT 5000
#define DEFAULT_SIZE 1000
#define DEFAULT_CONGESTION "cubic"

// Enum for congestion control algorithms
typedef enum
{
  RENO,
  CUBIC,
  VEGAS
} CongestionControl;

// Function that checks wether the given input is a number
bool isNumber(char *input)
{
  for (int i = 0; i < strlen(input); i++)
  {
    if (input[i] < '0' || input[i] > '9')
    {
      return false;
    }
  }
  return true;
}

// Function to convert string to congestion control algorithm
CongestionControl stringToCongestionControl(char *algo)
{
  if (strcmp(algo, "reno") == 0)
    return RENO;
  else if (strcmp(algo, "cubic") == 0)
    return CUBIC;
  else if (strcmp(algo, "vegas") == 0)
    return VEGAS;
  else
  {
    fprintf(stderr, "Invalid congestion control algorithm\n");
    exit(EXIT_FAILURE);
  }
}

// Function to convert congestion control algorithm to string
bool validatePort(int port)
{
  return port >= 0 && port <= 65535;
}

// Function to validate size
bool validateSize(int size)
{
  return size > 0;
}

// Read available system congestion control algorithms
char **readAvailableAlgorithms(int *count)
{
  FILE *file = fopen("/proc/sys/net/ipv4/tcp_available_congestion_control", "r");
  if (file == NULL)
  {
    perror("Error reading available congestion control algorithms");
    exit(EXIT_FAILURE);
  }

  char **algorithms = NULL;
  char *line = NULL;
  size_t len = 0;
  ssize_t read;
  int i = 0; // Number of elements in array

  // FIXME: This has to be fixed
  while ((read = getline(&line, &len, file)) != -1)
  {
    // Each algorithm is separated by a space
    char *token = strtok(line, " ");

    // For each token, add it to the array
    while (token != NULL)
    {
      // Remove newline character
      if (token[strlen(token) - 1] == '\n')
      {
        token[strlen(token) - 1] = '\0';
      }

      // Allocate memory for the algorithm and copy it to the array
      algorithms = (char **)realloc(algorithms, (i + 1) * sizeof(char *));
      algorithms[i] = (char *)malloc(strlen(token) + 1);
      strcpy(algorithms[i], token);
      token = strtok(NULL, " ");
      i++;
    }
  }

  fclose(file);
  if (line)
    free(line);

  *count = i;
  return algorithms;
}

bool validateHostname(char *hostname)
{
  // Ensure that the hostname is actually a valid hostname for DNS resolution
  // Use regex to check if it is a valid
  regex_t regex;
  int reti;
  reti = regcomp(&regex, "^[a-zA-Z0-9.-]*$", 0);

  // Check if the regex compiled successfully
  if (reti)
  {
    fprintf(stderr, "Could not compile regex\n");
    exit(EXIT_FAILURE);
  }

  // Execute the regex
  reti = regexec(&regex, hostname, 0, NULL, 0);

  // Free the memory allocated for the regex
  regfree(&regex);

  // Check if the regex matched
  return reti == 0;
}

void parseArgs(int argc, char *argv[], int *port, int *size, CongestionControl *c, char **ip_address)
{
  // Added a third option string for long options
  static struct option long_options[] = {
      {"port", required_argument, 0, 'p'},
      {"size", required_argument, 0, 'n'},
      {"congestion", required_argument, 0, 'c'},
      {"help", no_argument, 0, 'h'},
      {0, 0, 0, 0}};

  int long_index = 0;
  int opt;
  while ((opt = getopt_long(argc, argv, "hp:n:c:", long_options, &long_index)) != -1)
  {
    switch (opt)
    {
    case 'h':
      printf("Usage: %s [options] <server>\n", argv[0]);
      printf("\n");
      printf("Options:\n");
      printf("  -p, --port <port>          The port the server is listening on (default: 5000)\n");
      printf("  -n, --size <size>          The number of pseudo-random bytes to send to the server (default: 1000)\n");
      printf("  -c, --congestion <algo>    Congestion control algorithm to use (default: cubic)\n");
      printf("      Available algorithms: reno, cubic, vegas\n");
      printf("  -h, --help                 Display this help and exit\n");
      exit(EXIT_SUCCESS);
    case 'p':
      if (!isNumber(optarg))
      {
        fprintf(stderr, "Invalid port. Port must be an integer between 0 and 65535\n");
        exit(EXIT_FAILURE);
      }

      *port = atoi(optarg);

      // Validate port
      if (!validatePort(*port))
      {
        fprintf(stderr, "Invalid port. Port must be an integer between 0 and 65535\n");
        exit(EXIT_FAILURE);
      }
      break;
    case 'n':
      *size = atoi(optarg);
      // Validate size
      if (!validateSize(*size))
      {
        fprintf(stderr, "Invalid size. Size must be an integer greater than 0\n");
        exit(EXIT_FAILURE);
      }
      break;
    case 'c':
    {
      // Read available system TCP congestion control algorithms of the underlying system
      int count;
      char **algorithms = readAvailableAlgorithms(&count);

      // First we must check if the input is a congestion control algorithm supported by this software
      bool found = strcmp(optarg, "reno") == 0 || strcmp(optarg, "cubic") == 0 || strcmp(optarg, "vegas") == 0;
      if (!found)
      {
        // Notify user that the congestion control algorithm is not supported
        fprintf(stderr, "Selected congestion control algorithm is not supported. Please check supported algorithms using -h or --help option\n");
        exit(EXIT_FAILURE);
      }
      else
      {
        // If we have found the congestion control algorithm, then we must check if it is supported by the underlying system
        found = false; // Reuse same variable
        for (int i = 0; i < count; i++)
        {
          if (strcmp(algorithms[i], optarg) == 0)
          {
            // We set also the congestion control algorithm variable
            found = true;
            *c = stringToCongestionControl(optarg);
          }
        }
        if (!found)
        {
          // Notify user that the congestion control algorithm is not supported by its system
          printf("Selected congestion control algorithm is not available in your system. Have you Enabled TCP congestion algortihms in your system:\n");
          printf("List of enabled congestion control algorithms:\n");
          for (int i = 0; i < count; i++)
          {
            printf("\t- %s\n", algorithms[i]);
          }
          exit(EXIT_FAILURE);
        }
      }
      break;
    }
    default:
      exit(EXIT_FAILURE);
    }
  }
  if (optind < argc)
  {
    // Assuming the last argument is the server IP
    *ip_address = argv[optind];

    // Otherwise, check if it can be resolved
    struct sockaddr_in sa;
    if (inet_pton(AF_INET, *ip_address, &(sa.sin_addr)) == 0)
    {
      // if it is not an IP address, then we must check if its an hostname (i.e. localhost, luca.com, etc)
      bool isHostname = validateHostname(*ip_address);
      if (!isHostname)
      {
        fprintf(stderr, "Invalid server IP address or hostname. Have you typed it correctly?\n");
        exit(EXIT_FAILURE);
      }

      // Notify that we are going to resolve the hostname
      printf("Resolving hostname to IP address...");

      struct addrinfo hints, *res;
      memset(&hints, 0, sizeof hints);
      hints.ai_family = AF_UNSPEC;
      hints.ai_socktype = SOCK_STREAM;
      if (getaddrinfo(*ip_address, NULL, &hints, &res) != 0)
      {
        printf("fail\n");
        fprintf(stderr, "Invalid server IP address or hostname\n");
        exit(EXIT_FAILURE);
      }
      else
      {
        printf("done\n");
        // Get the first resolved IP address
        struct sockaddr_in *addr = (struct sockaddr_in *)res->ai_addr;
        *ip_address = inet_ntoa(addr->sin_addr);

        // Free the memory allocated by getaddrinfo
        freeaddrinfo(res);
      }
    }
  }
  else
  {
    fprintf(stderr, "Server IP address is required.\n");
    exit(EXIT_FAILURE);
  }
}

int main(int argc, char *argv[])
{
  int port = DEFAULT_PORT;
  int size = DEFAULT_SIZE;
  CongestionControl c = CUBIC;
  char *ip_address = NULL;

  // Parse and validate command line arguments
  parseArgs(argc, argv, &port, &size, &c, &ip_address);

  // Print used configuration
  printf("Configuration:\n");
  printf("\t- IP Address: %s\n", ip_address);
  printf("\t- Port: %d\n", port);
  printf("\t- Size: %d\n", size);
  printf("\t- Congestion Control: %s\n",
         c == RENO ? "reno" : c == CUBIC ? "cubic"
                                         : "vegas");
  // Generate random bytes to send to server
  srand(time(NULL));

  // Generate random bytes
  unsigned char *bytes = (unsigned char *)malloc(size);
  for (int i = 0; i < size; i++)
  {
    bytes[i] = rand() % 256;
  }

  // Create and save socket identifier
  printf("Creating socket...");
  int sockfd;
  if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
  {
    printf("fail\n");
    fprintf(stderr, "Error creating socket\n");
    return EXIT_FAILURE;
  }
  printf("done\n");

  // Set up server details
  printf("Setting up server details...");
  struct sockaddr_in server_address;
  server_address.sin_family = AF_INET;
  server_address.sin_port = htons(port);
  server_address.sin_addr.s_addr = inet_addr(ip_address);
  printf("done\n"); // Handle congestion control algorithms

  // Now, we need to set the congestion control algorithm
  printf("Setting congestion control algorithm...");
  int ret = 0;
  switch (c)
  {
  case RENO:
    ret = setsockopt(sockfd, IPPROTO_TCP, TCP_CONGESTION, "reno", sizeof("reno"));
    break;
  case CUBIC:
    ret = setsockopt(sockfd, IPPROTO_TCP, TCP_CONGESTION, "cubic", sizeof("cubic"));
    break;
  case VEGAS:
    ret = setsockopt(sockfd, IPPROTO_TCP, TCP_CONGESTION, "vegas", sizeof("vegas"));
    break;
  }
  if (ret < 0)
  {
    printf("fail\n");
    fprintf(stderr, "Error setting congestion control algorithm\n");
    return EXIT_FAILURE;
  }
  printf("done\n");

  // Connect to the server
  printf("Connecting to server...");
  if (connect(sockfd, (struct sockaddr *)&server_address, sizeof(server_address)) < 0)
  {
    printf("fail\n");
    fprintf(stderr, "Error while connecting to server\n");
    return EXIT_FAILURE;
  }
  printf("done\n");

  // Send bytes to server
  printf("Sending bytes to server...");
  if (send(sockfd, bytes, size, 0) < 0)
  {
    printf("fail\n");
    fprintf(stderr, "Error sending bytes to server");
    return EXIT_FAILURE;
  }
  printf("done\n");

  // Close the connection
  close(sockfd);

  // Set socket options for congestion control
  return EXIT_SUCCESS;
}