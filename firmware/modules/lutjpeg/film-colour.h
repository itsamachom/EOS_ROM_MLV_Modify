/* Integer display-only colour mapping. Never modifies a RAW or encoder buffer. */
static inline unsigned film_crc(unsigned crc,const unsigned char *p,unsigned n){while(n--){crc^=*p++;for(int k=0;k<8;k++)crc=(crc>>1)^((crc&1)?0xedb88320u:0);}return crc;}
static inline unsigned char film_index_pixel(const unsigned char *rgb){return 80+(rgb[0]*5+127)/255*24+(rgb[1]*5+127)/255*4+(rgb[2]*3+127)/255;}
static inline int film_clip(int x){return x<0?0:x>255?255:x;}
static inline void film_apply_uyvy(const uint32_t *src,uint32_t *dst,unsigned words,const uint32_t *table){
    for(unsigned i=0;i<words;i++){
        uint32_t p=src[i];int y0=(p>>8)&255,y1=p>>24,mean=(y0+y1)/2;
        unsigned index=(mean>>3)|(((p&255)>>3)<<5)|((((p>>16)&255)>>3)<<10);
        uint32_t color=table[index];int delta=(int)((color>>8)&255)-((mean&~7)+4);
        dst[i]=(color&0x00ff00ffu)|((uint32_t)film_clip(y0+delta)<<8)|((uint32_t)film_clip(y1+delta)<<24);
    }
}
static inline void film_uint(char *out,unsigned value){char reversed[12];int n=0;do{reversed[n++]=(char)('0'+value%10);value/=10;}while(value);for(int i=0;i<n;i++)out[i]=reversed[n-i-1];out[n]=0;}
