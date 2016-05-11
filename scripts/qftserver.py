import sys
import getopt
import quickftpy

#
# Callback logger function
#
def logger (f,m):
  print "[" + f + "]" + " - " + m
  return

#
# Main function
#
def main(argv):

  # Sets default values
  port=2332
  max_conn=128
  timeout=60000
  print ""

  # Parses parameters
  try:
    opts, args = getopt.getopt(argv,"hp:m:t:",["port=","max_conn=","timeout="])
  except getopt.GetoptError:
    print 'qftserver.py -p <port> -m <maxconnections> -t <timeout>'
    sys.exit(2)

  for opt, arg in opts:
    if opt == '-h':
      print 'qftserver.py -p <port> -m <maxconnections> -t <timeout>'
      sys.exit()
    elif opt in ("-p", "--port"):
      port = int(arg)
    elif opt in ("-m", "--max_conn"):
      max_conn = int(arg)
    elif opt in ("-m", "--max_conn"):
      max_conn = int(arg)

  # Initializes server
  quickftpy.servstart(port, max_conn, timeout, logger)

  print ""
  raw_input("Press Enter key at any moment to end execution...\n")

  # Finalizes server
  quickftpy.servend()

if __name__ == "__main__":
  main(sys.argv[1:])

