#include <vector>
#include <glm/glm.hpp>


const int width = 50;
const int height = 50;
const int depth = 50;

class
vectorField{
public:
  

  glm::vec3 field[depth][height][width];

  bool isInBounds(int x, int y, int z){
    return ((x >= 0)&&(x < width))&&((y >= 0)&&(y < height))&&((z >= 0)&&(z < depth));
  }
  
  glm::vec3 getValue(int x, int y, int z){
    assert(isInBounds(x, y, z));
    return field[z][y][x];
  }

  glm::vec3 getValue_boundedFallback(int x, int y, int z, glm::vec3 fallback){
    if(!isInBounds(x, y, z)){
      return fallback;
    }
    return getValue(x, y, z);
  }
  
  glm::vec3 calculateDerivative(int x, int y, int z, bool x_dir, bool y_dir, bool z_dir){
    assert(isInBounds(x, y, z));

    int dx = 0;
    int dy = 0;
    int dz = 0;
    if(x_dir){
      dx = 1;
    }
    if(y_dir){
      dy = 1;
    }
    if(z_dir){
      dz = 1;
    }

    int delta_position = dx+dy+dz;
    if(delta_position != 1){
      printf("derivative takes only only one axis at a time\n");
      assert(false);
    }
    
    glm::vec3 currVal = getValue(x, y, z);
    
    glm::vec3 pos = getValue_boundedFallback(x+dx, y+dy, z+dz, currVal);
    glm::vec3 neg = getValue_boundedFallback(x-dx, y-dy, z-dz, currVal);
    
    float real_delta_position = 2*delta_position;
    glm::vec3 derivative = (pos-neg)/real_delta_position;
    //(f(x+h) - f(x-h))/(x - (-x))
    //(f(x+h) - f(x-h))/2x
    
    return derivative;
  }
  
  glm::vec3 calculateCurl(int x, int y, int z){
    glm::vec3 derivX = calculateDerivative(x, y, z, 1, 0, 0);
    glm::vec3 derivY = calculateDerivative(x, y, z, 0, 1, 0);
    glm::vec3 derivZ = calculateDerivative(x, y, z, 0, 0, 1);

    glm::vec3 curl = glm::vec3(derivY.z - derivZ.y, derivZ.x - derivX.z, derivX.y - derivY.x);

    return curl;
  }
  
  vectorField(void){
    memset(&field, 0, sizeof(field));
  }
};


class
chargeParticle{
public:
  glm::vec3 pos;

  double charge;

  chargeParticle(glm::vec3 _pos, double _charge){
    pos = _pos;
    charge = _charge;
  }
};

class
electroMagneticField{
public:

  vectorField electricField;
  vectorField magneticField;
  
  
  
  electroMagneticField(void){
    
  }
};


class
simulation{
public:
  electroMagneticField EMField;
  std::vector<chargeParticle>  charges;

  void
  tick(float dt){
    for(int z = 0; z < depth; z++){
      for(int y = 0; y < height; y++){
	for(int x = 0; x < width; x++){
	  glm::vec3 dBdt = -EMField.electricField.calculateCurl(x, y, z);
	  EMField.magneticField.field[z][y][x] = dBdt*dt;
	}
      }
    }
  }

  simulation(void){

  }
};
