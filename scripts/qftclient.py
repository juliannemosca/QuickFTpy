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
  op_type = ""
  remote_filename = ""
  local_filename = ""
  addr = "127.0.0.1"
  port="2332"
  timeout=20000
  timeout_ack=15000

  result = -1
  usage="qftclient.py -o <operation type: receive, send, delete> -r " +
        "<remote filename> -l <local filename> -a <server address> -p " +
        "<server port> -t <messages timeout> -k <ack timeout>"
  print ""

  # Parses parameters
  try:
    opts, args = getopt.getopt(argv,"ho:r:l:a:p:t:k:",[
                                               "operation="
                                               "remotefile=",
                                               "localfile=",
                                               "addr=",
                                               "port=",
                                               "timout=",
                                               "tack="])
  except getopt.GetoptError:
    print "%s" % usage
    sys.exit(2)

  for opt, arg in opts:
    if opt == '-h':
      print "%s" % usage
      sys.exit()
    elif opt in ("-o", "--operation"):
      op_type = arg
    elif opt in ("-r", "--remotefile"):
      remote_filename = arg
    elif opt in ("-l", "--localfile"):
      local_filename = arg
    elif opt in ("-a", "--addr"):
      addr = arg
    elif opt in ("-p", "--port"):
      port = arg
    elif opt in ("-t", "--timeout"):
      timeout = int(arg)
    elif opt in ("-k", "--tack"):
      timeout_ack = int(arg)

  # Checks for required parameters and performs operations
  if op_type == "send":

    if (remote_filename == "") or (local_filename == ""):
      print "%s" % usage
      sys.exit()

    # Performs File Send operation
    result = quickftpy.clsend(remote_filename, local_filename, addr, port, timeout, timeout_ack, logger)

  elif op_type == "receive":

    if (remote_filename == "") or (local_filename == ""):
      print "%s" % usage
      sys.exit()
    
    # Performs File Receive operation
    result = quickftpy.clrecv(remote_filename, local_filename, addr, port, timeout, timeout_ack, logger)

  elif op_type == "delete":

    if remote_filename == "":
      print "%s" % usage
      sys.exit()

    # Performs File Delete operation
    result = quickftpy.cldel(remote_filename, addr, port, timeout, timeout_ack, logger)

  else:
    print "operation not supported.\n"
    print "%s" % usage
    sys.exit()
  
  print "\noperation result: %d\n" % result
  print ""
  raw_input("Press Enter key to finalize...\n")

if __name__ == "__main__":
  main(sys.argv[1:])

