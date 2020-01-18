#include <iostream>
#include <string>
#include <iterator>
#include <algorithm>
#include <vector>
#include <assert.h>
#include <memory.h>
#include <math.h>

//
// Computer Vision experimentation + template programming practice
// 

///
/// @brief A custom range.  Supports comparison and build increment
///
template<class T, class ConstT>
class Myrange
{
  public: 

  using iterator = T;
  using const_iterator = ConstT;

  Myrange( iterator b, iterator e ) :
    begin_iter{ b }, end_iter{ e } {};

  Myrange() :
    begin_iter{ iterator() }, end_iter{ iterator() } {};

  iterator begin() {
    return begin_iter;
  }
  iterator end() {
    return end_iter;
  }
  const_iterator cbegin() {
    return begin_iter;
  }
  const_iterator cend() {
    return end_iter;
  }

  Myrange& operator+=( int num )
  {
    begin_iter += num;
    end_iter += num;
    return *this;
  }

  Myrange& operator-=( int num )
  {
    begin_iter -= num;
    end_iter -= num;
    return *this;
  }

  bool operator==( const Myrange& rhs ) const
  {
    return begin_iter == rhs.begin_iter && end_iter == rhs.end_iter;
  }

  private:

  iterator begin_iter;
  iterator end_iter;
};

///
/// @brief Y Iterator for an image class.  Dereferencing the iterator 
/// returns a range of pixels that represent a row in the image.
///
/// @param[in] T       - The Image Type
/// @param[in] isConst - true if this should be a constant iterator. 
///                      modification of the ranges isn't allowed because
///                      the range is stored inside the iterator, so all
///                      iterators are really constant iterators.
/// 
template<class T, bool isConst>
class YIter_Base
{
  public:

  using x_iterator = typename std::conditional< isConst, 
    typename T::const_iterator, 
    typename T::iterator>::type;
  using x_const_iterator = typename T::const_iterator;
  using value_type = typename std::conditional< isConst, 
      Myrange< x_const_iterator, x_const_iterator >,
      Myrange< x_iterator, x_const_iterator >>::type;
  using iterator_category = std::forward_iterator_tag;
  using iterator = YIter_Base;
  using reference = typename std::conditional< isConst, 
      value_type, 
      const value_type &>::type;
  using pointer = const value_type *;

  /// @brief Constructor for users
  ///
  /// @param[in] s      The image (x) iterator that starts the range
  /// @param[in] span   The number of pixels in a row
  /// @param[in] y      The row count.  TODO - probably remove
  ///
  YIter_Base( x_iterator s, size_t span, size_t y ) :
    x_range{ s, s+span },
    x_span { span }, 
    y_val { y } 
  {
  }

  ///////////////////////////////////////////////////////////////////
  //
  // Interfaces added for general iterator compliance
  //
  ///////////////////////////////////////////////////////////////////

  /// @brief Constructor
  YIter_Base() : x_range{}, x_span{0}, y_val{ 0 } {}  
  /// @brief Copy Constructor
  YIter_Base( const iterator& ) = default;
  /// @brief Destructor
  ~YIter_Base() = default;
  /// @brief Assignment Operator
  iterator& operator=( const iterator& ) = default;

  /// @brief Increment operator
  iterator& operator++()
  {
    x_range += x_span;
    ++y_val;
    return *this;
  }

  // Only enable if the reference isn't value_type
  template <typename my_reference,
    std::enable_if_t< !std::is_same<my_reference, value_type>::value, reference> = 0 >
  my_reference operator*() const
  {
    return x_range;
  }

  // TODO - swap.

  ///////////////////////////////////////////////////////////////////
  //
  // Interfaces added for Input iterator compliance
  //
  ///////////////////////////////////////////////////////////////////

  iterator operator++(int)  // postfix increment
  {
    iterator rval{ *this };
    ++rval;
    return rval;
  }

  value_type operator*() const
  {
    return x_range;
  }
  size_t getY() const { return y_val; } 

  pointer operator->() const = delete;

  bool operator==( const iterator& rhs ) const
  {
    return x_range == rhs.x_range && x_span == rhs.x_span && y_val == rhs.y_val;
  }

  bool operator!=( const iterator& rhs ) const
  {
    return !(*this == rhs );
  }

  //
  // Mostly added for convienence.
  // 

  /// @brief Addtion operator
  iterator& operator+=( size_t t)
  {
    x_range += t * x_span;
    y_val += t;
    return *this;
  }
  /// @brief Addition operator
  iterator operator+( size_t t )
  {
    iterator rval = *this;
    rval+=t;
    return rval;
  }
  /// @brief Subtraction operator
  iterator& operator-=( size_t t)
  {
    x_range -= t * x_span;
    y_val -= t;
    return *this;
  }
  /// @brief Subtraction operator
  iterator operator-( size_t t )
  {
    iterator rval= *this;
    rval-=t;
    return rval;
  }


  private:
  value_type x_range;
  const size_t x_span;
  size_t y_val;
};

template<class T>
class Image
{
  public: 

  using raw_storage = std::vector<T>;
  using value_type = T;
  using reference = T&;
  using const_reference = const T&;
  using pointer = T*;
  using const_pointer = const T*;
  using iterator = typename raw_storage::iterator;
  using const_iterator = typename raw_storage::const_iterator;
  using difference_type = typename std::iterator_traits<iterator>::difference_type;
  using size_type = std::size_t;
  using allocator_type = typename raw_storage::allocator_type;
  using x_range = Myrange< iterator, const_iterator >; 
  using yiter = YIter_Base< raw_storage, false >;
  using const_yiter = YIter_Base< raw_storage, true >;
  using y_range = Myrange< yiter, const_yiter >;
  using const_y_range = Myrange< const_yiter, const_yiter >;

  Image( size_t x, size_t y ) : 
    xdim{ x }, 
    ydim{ y }, 
    raw_pixels{ x * y, allocator_type() } {}

  Image() :   
    xdim{0}, 
    ydim{0}, 
    raw_pixels{ 0, allocator_type() } {}

  Image( Image&& o ) noexcept : 
    xdim{ std::exchange(o.xdim, xdim )}, 
    ydim{ std::exchange(o.ydim, ydim )}, 
    raw_pixels{ std::move( o.raw_pixels )} {}; 

  Image& operator=( Image&& o ) noexcept 
  {
    xdim = std::exchange(o.xdim, xdim ); 
    ydim = std::exchange(o.ydim, ydim );
    raw_pixels = std::move( o.raw_pixels );
  }

  Image( const Image& other ) = default;

  void swap( Image& other ) {
    raw_pixels.swap( other.raw_pixels );
    std::swap( xdim, other.xdim );
    std::swap( ydim, other.ydim );
  }

  iterator begin() {
    return raw_pixels.begin();
  }
  iterator end() {
    return raw_pixels.end();
  }
  const_iterator cbegin() const {
    return raw_pixels.cbegin();
  }
  const_iterator cend() const {
    return raw_pixels.cend();
  }

  yiter ybegin()
  {
    return yiter{ raw_pixels.begin(), xdim, 0 }; 
  }
  yiter yend()
  {
    return yiter{ raw_pixels.begin() + xdim * ydim, xdim, ydim }; 
  }
  const_yiter ycbegin() const
  {
    return const_yiter{ raw_pixels.cbegin(), xdim, 0 }; 
  }
  const_yiter ycend() const
  {
    return const_yiter{ raw_pixels.cbegin() + xdim * ydim, xdim, ydim }; 
  }

  y_range yrange()
  {
    return y_range{ ybegin(), yend() };
  }

  T& at( size_t y, size_t x ) {
    assert( x >= 0 && x < xdim );
    assert( y >= 0 && y < ydim );
    return raw_pixels[ x + y * xdim ];
  }

  size_t get_x_dim() const { return xdim; }
  size_t get_y_dim() const { return ydim; }

  private:

  size_t xdim, ydim;
  std::vector<T> raw_pixels;
};

template<class InputIt, class OutputIt, class BinaryOperation>
OutputIt transform_ditr(
  InputIt first1, InputIt last1, 
  OutputIt d_first, BinaryOperation binary_op)
{
    while (first1 != last1) {
        binary_op(*d_first, *first1);
        ++d_first;
        ++first1;
    }
    return d_first;
}

template<class DstRange, class SrcRange>
void x_edge_detect_for_row( DstRange dst_range, SrcRange src_range )
{
  std::transform( 
      src_range.cbegin(), src_range.cend()-2, // left pixels
      src_range.cbegin()+2,                   // right pixels
      dst_range.begin()+1,                    // dst pixels
      []( auto left, auto right ) { return std::abs( right - left ); } );
}

template<class TImage>
TImage x_edge_detect( const TImage& src )
{
  TImage dst( src.get_x_dim(), src.get_y_dim());

  transform_ditr( 
      src.ycbegin(), src.ycend(),            // Source row ranges
      dst.ybegin(),                          // Dstination row ranges
      []( const auto& dst_x_range, const auto& src_x_range ) {
        x_edge_detect_for_row( dst_x_range, src_x_range ); 
      });

  return dst;
}

template<class DstRange, class SrcRange>
void y_edge_detect_for_row( 
  DstRange dst_range, 
  SrcRange src_above_range, 
  SrcRange src_below_range 
) 
{
  std::transform( 
      src_above_range.cbegin(), src_above_range.cend(),  // row @ y-1
      src_below_range.cbegin(),                          // row @ y+1
      dst_range.begin()+1,                               // dst row
      []( auto above, auto below ) { return std::abs( above - below ); } );
}

template<class TImage>
TImage y_edge_detect( const TImage& src )
{
  TImage dst( src.get_x_dim(), src.get_y_dim());

  auto dst_itr = dst.ybegin()+1;
  auto dst_end = dst.yend()-1;
  auto src_above = src.ycbegin();
  auto src_below = src.ycbegin()+2;

  int y = 1;
  while ( dst_itr != dst_end ) {
    y_edge_detect_for_row( *dst_itr, *src_above, *src_below );
    ++dst_itr;
    ++src_above;
    ++src_below;
  }
  return dst;
}

template<class DstImage, class SrcImage>
DstImage convert( const SrcImage& src, float scale )
{
  DstImage dst{ src.get_x_dim(), src.get_y_dim() };

  std::transform( src.cbegin(), src.cend(), dst.begin(), [scale]( auto in ) 
  {
    typename DstImage::value_type raw_out = (typename DstImage::value_type) in;
    return raw_out * scale;
  });

  return dst;
}

using ImageF = Image<float>;
using ImageUC = Image<unsigned char>;

ImageUC load_pgm_from_stdin()
{
  std::string line;
  int xdim, ydim;

  std::cin >> line;
  std::cin >> xdim >> ydim;
  std::cin >> line;

  ImageUC ucimage( xdim, ydim );
  for ( auto &p : ucimage )
  {
    p = getchar();
  }
  return ucimage;
}

ImageF hough( ImageF &im, float ntx = 720, float mry = 720)
{
  ImageF h( ntx, mry );

  float nimx = im.get_x_dim();
  float nimy = im.get_y_dim();

  float rmax = sqrtf( nimx * nimx + nimy * nimy );
  float dr = rmax / ( mry / 2.0f );
  float dth = M_PI / ntx;

  float iy = 0.0f;
  for ( auto x_range : im.yrange() )
  {
    float jx = 0.0f;
    for ( const auto& pixel : x_range ) 
    {
      if ( pixel >= 0.5f ) {
      for ( float jtx = 0; jtx < ntx; ++jtx )
      {
        float vx = jx - nimx/2;
        float vy = -(iy - nimy/2);
        vx = jx;
        vy = -iy;

        float th = dth * jtx;
        //float vx = cosf( th );
        //float vy = sinf( th );
        //float r = std::abs( vx * cosf( th ) - vy * sinf( th ));
        float r = vx * sinf( th ) - vy * cosf( th );
        //std::cerr << th << " " << jx << " " << iy << " " << r << "\n";
        float iry = mry / 2.0f + r/dr;
        //float iry = mry-1 - r/dr*2;
        h.at( (int) iry, (int) jtx ) += 0.003f;     
      }
      }

      ++jx;
    }
    ++iy;
  }
  return h;
}


main()
{
  ImageUC loaded_image = load_pgm_from_stdin(); 
  auto fimage = convert<ImageF>( loaded_image, 1.0f/255.0f );
  auto xedge = x_edge_detect( fimage );
  auto yedge = y_edge_detect( fimage );


  ImageF combo( xedge.get_x_dim(), xedge.get_y_dim());

  std::transform( xedge.begin(), xedge.end(), yedge.begin(), combo.begin(),
    []( float a, float b ) { return std::min( a+b, 1.0f ); } );

  std::transform( combo.begin(), combo.end(), combo.begin(),
    []( float a ) { return a > 0.4f ? 1.0f : 0.0f; } );

  auto h = hough( combo );
  float maxp = 0;
  for ( auto& p : h ) {
    maxp = std::max( p, maxp );
  }
  std::transform( h.begin(), h.end(), h.begin(),
    [maxp]( float c ) -> float { return c / maxp; } );

#define DO_H
//#define DO_EDGE

#ifdef DO_H
  ImageUC outImage( h.get_x_dim(), h.get_y_dim() );
  std::transform( h.begin(), h.end(), outImage.begin(),
    []( float c ) -> unsigned char { return (unsigned char) ( c * 255.0f ); });
#endif

#ifdef DO_EDGE
  ImageUC outImage( combo.get_x_dim(), combo.get_y_dim() );
  std::transform( combo.begin(), combo.end(), outImage.begin(),
    []( float c ) -> unsigned char { return (unsigned char) ( c * 255.0f ); });
#endif

  std::cout << "P5\n";
  std::cout << outImage.get_x_dim() << " " << outImage.get_y_dim() << "\n";
  std::cout << "255\n";
  for ( auto& c : outImage ) {
    putchar( c );
  }
 
}

