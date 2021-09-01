
import numbers
import math

class GeoUtil:
  @staticmethod
  def degree2radius(degree):
    return degree * (math.pi/180)

  @staticmethod
  def get_harversion_distance(x1,y1,  x2,y2, round_decimal_digits=5):
    """
    경위도 (x1,y1)과 (x2,y2) 점의 거리를 반환
    harversion formula 이용하여 2개의 경위도간 거리를 구함(단위:km)
    """

    if x1 is None or y1 is None or x2 is None or y2 is None:
      return None

    assert isinstance(x1, numbers.Number) and -180 <= x1 and x1 <= 180
    assert isinstance(y1, numbers.Number) and -90 <= y1 and y1 <= 90
    assert isinstance(x2, numbers.Number) and -180 <= x2 and x2 <= 180
    assert isinstance(y2, numbers.Number) and -90 <= y2 and y2<= 90

    R = 6371 # 지구의 반경(단위: km)

    dLon = GeoUtil.degree2radius(x2-x1)
    dLat = GeoUtil.degree2radius(y2-y1)

    a = math.sin(dLat/2) * math.sin(dLat/2) \
      + (math.cos(GeoUtil.degree2radius(y1))  \
        *math.cos(GeoUtil.degree2radius(y2))  \
        *math.sin(dLon/2) * math.sin(dLon/2))

    b = 2 * math.atan2(math.sqrt(a), math.sqrt(1-a))

    return  round( R * b, round_decimal_digits)


  @staticmethod
  def get_euclidean_distance(x1,y1, x2,y2, round_decimal_digits=5):
    """
    유클리안 formula 이용하여 (x1,y1)과 (x2,y2)점의 거리를 반환
    """

    if x1 is None or y1 is None or x2 is None or y2 is None:
      return None

    assert isinstance(x1, numbers.Number) and -180 <= x1 and x1 <= 180
    assert isinstance(y1, numbers.Number) and -90 <= y1 and y1 <= 90
    assert isinstance(x2, numbers.Number) and -180 <= x2 and x2 <= 180
    assert isinstance(y2, numbers.Number) and -90 <= y2 and y2<= 90


    dLon = abs(x2-x1)  # 경도 차이
    if dLon >= 180:
      dLon -= 360

    dLat = y2-y1  # 위도 차이

    return round( math.sqrt(pow(dLon,2) + pow(dLat,2)), round_decimal_digits)




camera_pos  = {
(35.139425,128.820543):(190,50),
(35.327421,129.046001):(180,50),
(35.266681,128.750388):(90,50)
}

b = camera_pos[35.139425,128.820543]