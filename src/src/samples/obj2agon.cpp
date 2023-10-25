// obj2agon.c - convert OBJ model to C source file
//
// Linux program compilation:
// g++ obj2agon.cpp -o obj2agon
//
// g obj1
// v 21.333336 -24.500193 5
// f 1 2 3

#include <stdio.h>
#include <stdint.h>
#include <vector>

typedef struct {
  float x;
  float y;
  float z;
} Vertex;

typedef struct {
  uint16_t ix;
  uint16_t iy;
  uint16_t iz;
} Face;

std::vector<Vertex> vertices;
std::vector<Face> faces;


int flush_object() {
  if (!vertices.empty() && !faces.empty()) {
    printf("%lu vertices\n", vertices.size());
    printf("%lu faces\n", faces.size());
  }
  vertices.clear();
  faces.clear();
  return 0;
}

int convert_data(FILE* fin, FILE* fout) {
  char line[200];
  int cnt = 0;
  while (fgets(line, sizeof(line)-1, fin)) {
    cnt++;
    if (line[0] == 'g') {
      int rc = flush_object();
      if (!rc) {
        return rc;
      }
    } else if (line[0] == 'v') {
      Vertex v { 0.0f, 0.0f, 0.0f };
      if (3 == sscanf("%f %f %f", line, &v.x, &v.y, &v.z)) {
        printf("*");
        vertices.push_back(v);
      } else {
        printf("Invalid line: %s", line);
        return 4;
      }
    } else if (line[0] == 'f') {
      Face f { 0, 0, 0 };
      if (3 == sscanf("%hu %hu %hu", line, &f.ix, &f.iy, &f.iz)) {
        printf(".");
        faces.push_back(f);
      } else {
        printf("Invalid line: %s", line);
        return 5;
      }
    } else {
      printf("%s", line);
    }
  }
  printf("%u lines\n", cnt);
  return flush_object();
}

int main(int argc, const char* argv[]) {
  if (argc != 3) {
    printf("Use: obj2agon input-file output-file\n");
    return 1;
  } else {
    FILE* fin = fopen(argv[1], "r");
    if (fin) {
      printf("Reading: %s\n", argv[1]);
      FILE* fout = fopen(argv[2], "w");
      if (fout) {
        printf("Writing: %s\n", argv[2]);
        int rc = convert_data(fin, fout);
        fclose(fout);
        fclose(fin);
        return rc;
      } else {
        printf("Can't open output file");
        fclose(fin);
        return 3;
      }
    } else {
      printf("Can't open input file");
      return 2;
    }
  }
}
