#include <big_mpi_compat.h>

#include <cassert>
#include <iostream>
#include <vector>

void
test_write(const std::uint64_t n_bytes, const std::string &command)
{
  int ierr;
  int myid, ranks;
  MPI_Comm_rank(MPI_COMM_WORLD, &myid);
  MPI_Comm_size(MPI_COMM_WORLD, &ranks);
  assert(ranks == 2);

  std::uint64_t offset = myid * n_bytes;

  MPI_File fh;
  MPI_Info info;
  ierr = MPI_Info_create(&info);
  CheckMPIFatal(ierr);

  ierr = MPI_File_open(
    MPI_COMM_WORLD, "io.data", MPI_MODE_CREATE | MPI_MODE_WRONLY, info, &fh);
  CheckMPIFatal(ierr);

  std::vector<char> buffer(n_bytes, '?');
  buffer[0] = 'A' + myid;

  if (command == "write_at")
    ierr = BigMPICompat::MPI_File_write_at_c(
      fh, offset, buffer.data(), buffer.size(), MPI_CHAR, MPI_STATUS_IGNORE);
  else if (command == "write_at_all")
    ierr = BigMPICompat::MPI_File_write_at_all_c(
      fh, offset, buffer.data(), buffer.size(), MPI_CHAR, MPI_STATUS_IGNORE);
  else if (command == "write_ordered")
    {
      (void)offset;
      ierr = BigMPICompat::MPI_File_write_ordered_c(
        fh, buffer.data(), buffer.size(), MPI_CHAR, MPI_STATUS_IGNORE);
    }
  else
    MPI_Abort(MPI_COMM_WORLD, 1);

  CheckMPIFatal(ierr);

  ierr = MPI_File_sync(fh);
  CheckMPIFatal(ierr);

  MPI_Barrier(MPI_COMM_WORLD);

  if (myid == 0)
    {
      int result =
        system("md5sum io.data | grep 4c1fe47ac9c80d7af0ea289c90e13132");
      if (result == 0)
        std::cout << "io: " << command << " md5sum: OK" << std::endl;
      else
        {
          std::cerr << "io: " << command
                    << " md5sum FAILED - result: " << result << std::endl;
          MPI_Abort(MPI_COMM_WORLD, 1);
        }
    }

  ierr = MPI_File_close(&fh);
  CheckMPIFatal(ierr);
}


void
test_read(const std::uint64_t n_bytes, const std::string &command)
{
  int ierr;
  int myid, ranks;
  MPI_Comm_rank(MPI_COMM_WORLD, &myid);
  MPI_Comm_size(MPI_COMM_WORLD, &ranks);
  assert(ranks == 2);

  std::uint64_t offset = myid * n_bytes;

  MPI_File fh;
  MPI_Info info;
  ierr = MPI_Info_create(&info);
  CheckMPIFatal(ierr);

  ierr = MPI_File_open(MPI_COMM_WORLD,
                       "io.data",
                       MPI_MODE_DELETE_ON_CLOSE | MPI_MODE_RDONLY,
                       info,
                       &fh);
  CheckMPIFatal(ierr);

  std::vector<char> buffer(n_bytes, '?');

  if (command == "read_at")
    ierr = BigMPICompat::MPI_File_read_at_c(
      fh, offset, buffer.data(), buffer.size(), MPI_CHAR, MPI_STATUS_IGNORE);
  else if (command == "read_at_all")
    ierr = BigMPICompat::MPI_File_read_at_all_c(
      fh, offset, buffer.data(), buffer.size(), MPI_CHAR, MPI_STATUS_IGNORE);
  else
    MPI_Abort(MPI_COMM_WORLD, 1);

  CheckMPIFatal(ierr);

  // ierr = MPI_File_sync(fh);
  // CheckMPIFatal(ierr);

  MPI_Barrier(MPI_COMM_WORLD);

  ierr = MPI_File_close(&fh);
  CheckMPIFatal(ierr);
}

int
main(int argc, char *argv[])
{
  MPI_Init(&argc, &argv);

  test_write((1ULL << 32) + 2, "write_at");
  std::cout << "in script write at done. " << std::endl;

  test_read((1ULL << 32) + 2, "read_at");
  std::cout << "in script read at done. " << std::endl;

  test_write((1ULL << 32) + 2, "write_at_all");
  std::cout << "in script write at all done. " << std::endl;

  test_read((1ULL << 32) + 2, "read_at_all");
  std::cout << "in script read at all done. " << std::endl;

  MPI_Finalize();
  return 0;
}
