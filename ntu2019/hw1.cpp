#include <iostream>
#include <algorithm>
#include <vector>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "jbig.h"

#define WIDTH 16
#define HEIGHT 16

static int total_len;
static void data_out(unsigned char *start, size_t len, void *file)
{
    total_len += len;
   
    if( file != NULL ) fwrite(start, len, 1, (FILE *)file);
    return;
}


int encodeJBIG(unsigned char* original, size_t size,
               int options, int order, int layers,
               unsigned long l0, int mx,
               char* output_file_name)
{
    struct jbg_enc_state status;

    unsigned char **image;
    image = (unsigned char **) malloc( sizeof(unsigned char *) ) ;
    image[0] = (unsigned char *) malloc(size);
    memcpy(image[0], original, size);

    FILE *fout = stdout;
    if( output_file_name != NULL )
    {
        fout = fopen(output_file_name, "wb");
        if( !fout )
        {
            std::cout << "cannot open file" << std::endl;
            exit(1);
        }
        jbg_enc_init(&status, WIDTH, HEIGHT, 1, image, data_out, fout);
    }
    else
    {
        jbg_enc_init(&status, WIDTH, HEIGHT, 1, image, data_out, NULL);
    }

    jbg_enc_layers(&status, layers);
    jbg_enc_options(&status, order, options, l0, mx, 0);
    jbg_enc_out(&status);
    jbg_enc_free(&status);
    
    free(image[0]);
    free(image);
    if( output_file_name != NULL ) fclose(fout);
}


void combination(int n, int m, unsigned char *img, size_t size)
{
    std::vector<int> list;
    std::vector<int> a;

    int  out_idx = 0;
    char out_name[20];

    int max_value = 0;
    int min_value = 1000;
    for( int i=0; i<n; i++ )
    {
        list.push_back(i+1);
    }

    for( int i=0; i<m; i++ )
    {
        a.push_back(i+1);
    }

    for( int j=m; a[0]<=(n-m+1); )
    {
        for(; a[m-1]<=n; a[m-1]++)
        {

            // print pattern
            bool data[WIDTH][HEIGHT];
            for( size_t k=0; k<WIDTH; k++ )
            {
                for( size_t l=0; l<HEIGHT; l++ )
                {
                    data[k][l] = 0;
                }
            }
            for( int t=0; t<m; t++ )
            {
                int k = (list[a[t]-1]-1)%WIDTH;
                int l = (list[a[t]-1]-1)/WIDTH;
                data[k][l] = 1;
            }

            // generate img
            size_t idx = 0;
            for( size_t l=0; l<HEIGHT; l++ )
            {
                for( size_t k=0; k<((WIDTH+7)/8); k++ )
                {
                    // parse binary into char
                    unsigned char c = 0;
                    for( size_t h=k*8; h<(k+1)*8 && h<WIDTH; h++ )
                    {
                        if( data[h][l] ) c |= 1 << (h-k*8);
                    }
                    img[idx] = c;
                    idx ++;
                }
            }

            total_len = 0;
            // opitons=JBG_DELAY_AT, order=0, layers=0, l0=16, mx=0
            encodeJBIG(img, size, JBG_DELAY_AT, 0, 0, WIDTH, 0, NULL);

            if( total_len<min_value )
            {
                min_value = total_len;

                for( size_t l=0; l<HEIGHT; l++ )
                {
                    for( size_t k=0; k<WIDTH; k++ )
                    {
                        if( data[k][l] == 1 ) std::cout << "#" ;
                        else std::cout << "-";
                    }
                    std::cout << std::endl;
                }
                std::cout << std::endl << "LEN=" << total_len << std::endl << std::endl;

                sprintf( out_name, "out_%03d.jbg", out_idx);
                encodeJBIG(img, size, JBG_DELAY_AT, 0, 0, WIDTH, 0, out_name);
                out_idx ++;
            }
            else if( total_len>max_value )
            {
                max_value = total_len;

                for( size_t l=0; l<HEIGHT; l++ )
                {
                    for( size_t k=0; k<WIDTH; k++ )
                    {
                        if( data[k][l] == 1 ) std::cout << "#" ;
                        else std::cout << "-";
                    }
                    std::cout << std::endl;
                }
                std::cout << std::endl << "LEN=" << total_len << std::endl << std::endl;
                sprintf( out_name, "out_%03d.jbg", out_idx);
                encodeJBIG(img, size, JBG_DELAY_AT, 0, 0, WIDTH, 0, out_name);
                out_idx ++;
            }
        }

        for( j=m-2; j>=0; j-- )
        {
            a[j]++;
            if( a[j]<=(j+n-m+1) ) break;
        }

        for( j++; j>0 && j<m; j++ )
        {
            a[j] = a[j-1]+1;
        }
    }

    std::cout << "minimal encoded size is " << min_value << std::endl;
    std::cout << "maximal encoded size is " << max_value << std::endl;
}

bool seed_comp(double i, double j){ return (i<j); }

void rand_sample(unsigned char *img, size_t size, size_t times)
{
    int  out_idx = 0;
    char out_name[20];

    int max_value = 0;
    int min_value = 1000;

    unsigned seed;
    seed = (unsigned)time(NULL);
    srand(seed);

    size_t m = WIDTH*HEIGHT*0.5;

    for( size_t t=0; t<times; t++ )
    {
        std::vector<double> list;
        std::vector<double> temp;

        for( size_t i=0; i<WIDTH*HEIGHT; i++ )
        {
            double v = ( (double)rand() / (RAND_MAX) ) ;
            list.push_back(v);
            temp.push_back(v);
        }

        std::sort( temp.begin(), temp.end(), seed_comp);
        double threshold = temp[m];

        // generate img
        size_t idx = 0;
        for( size_t l=0; l<HEIGHT; l++ )
        {
            for( size_t k=0; k<((WIDTH+7)/8); k++ )
            {
                // parse binary into char
                unsigned char c = 0;
                for( size_t h=k*8; h<(k+1)*8 && h<WIDTH; h++ )
                {
                    if( list[l*WIDTH+h]<threshold )
                        c |= 1 << (h-k*8);
                }

                img[idx] = c;
                idx ++;
            }
        }

        total_len = 0;
        // opitons=JBG_DELAY_AT, order=0, layers=0, l0=16, mx=0
        encodeJBIG(img, size, JBG_DELAY_AT, 0, 0, WIDTH, 0, NULL);


        if( total_len<min_value )
        {
            min_value = total_len;

            for( size_t l=0; l<HEIGHT; l++ )
            {
                for( size_t k=0; k<WIDTH; k++ )
                {
                    if( list[l*WIDTH+k]<threshold ) std::cout << "#" ;
                    else std::cout << "-";
                }
                std::cout << std::endl;
            }
            std::cout << std::endl << "LEN=" << total_len << std::endl << std::endl;

            sprintf( out_name, "out_%03d.jbg", out_idx);
            encodeJBIG(img, size, JBG_DELAY_AT, 0, 0, WIDTH, 0, out_name);
            out_idx ++;
        }
        else if( total_len>max_value )
        {
            max_value = total_len;

            for( size_t l=0; l<HEIGHT; l++ )
            {
                for( size_t k=0; k<WIDTH; k++ )
                {
                    if( list[l*WIDTH+k]<threshold ) std::cout << "#" ;
                    else std::cout << "-";
                }
                std::cout << std::endl;
            }
            std::cout << std::endl << "LEN=" << total_len << std::endl << std::endl;
            sprintf( out_name, "out_%03d.jbg", out_idx);
            encodeJBIG(img, size, JBG_DELAY_AT, 0, 0, WIDTH, 0, out_name);
            out_idx ++;
        }
    }
    std::cout << "minimal encoded size is " << min_value << std::endl;
    std::cout << "maximal encoded size is " << max_value << std::endl;
}


int main(int argc, char **argv)
{
    std::cout << "\e[1mHomework 1.\e[0m" << std::endl;
    std::cout << "\e[3mChapter 2. Binary image coding\e[0m" << std::endl;
    std::cout << "Images (assuming some viewing condition) and show the matrix and compression ratio.";
    std::cout << "Consider a 16x16 binary image with the same number of black and white pixels inside (128 for each). What ";
    std::cout << "block pattern will make JBIG-1 encoder perform best? What will be the worst?" << std::endl << std::endl;


    /* Initialization */
    unsigned char *img;
    size_t size = ((WIDTH + 7) / 8) * HEIGHT;
    img = (unsigned char *) malloc( size*sizeof(unsigned char) );

    if( argc < 2 )
    {
        /* Testing different pattern */
        // p(white)=p(black)=0.5
        combination(WIDTH*HEIGHT, WIDTH*HEIGHT*0.5, img, size);
    }
    else if( argc < 3 )
    {
        rand_sample(img, size, atoi(argv[1]));
    }
    else if( argc==int(size+2) )
    {
        // user set pattern
        for( size_t i=0; i< size; i++ )
        {
            img[i] = atoi(argv[i+1]);
        }
        encodeJBIG(img, size, JBG_DELAY_AT, 0, 0, WIDTH, 0, argv[1+size] );
    }

    free(img);
    return 0;
}
