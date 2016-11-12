#include "../main.h"

#include "Deflate.h"

typedef struct uivector
{
  unsigned* data;
  size_t size;
  size_t allocsize;
} uivector;

static void uivector_cleanup(void* p)
{
  ((uivector*)p)->size = ((uivector*)p)->allocsize = 0;
  free(((uivector*)p)->data);
  ((uivector*)p)->data = nullptr;
}

static unsigned uivector_reserve(uivector* p, size_t allocsize)
{
  if(allocsize > p->allocsize)
  {
    size_t newsize = (allocsize > p->allocsize * 2) ? allocsize : (allocsize * 3 / 2);
    void* data = realloc(p->data, newsize);
    if(data)
    {
      p->allocsize = newsize;
      p->data = (unsigned*)data;
    }
    else return 0;
  }
  return 1;
}

static unsigned uivector_resize(uivector* p, size_t size)
{
  if(!uivector_reserve(p, size * sizeof(unsigned))) return 0;
  p->size = size;
  return 1;
}

static unsigned uivector_resizev(uivector* p, size_t size, unsigned value)
{
  size_t oldsize = p->size, i;
  if(!uivector_resize(p, size)) return 0;
  for(i = oldsize; i < size; ++i) p->data[i] = value;
  return 1;
}

static void uivector_init(uivector* p)
{
  p->data = nullptr;
  p->size = p->allocsize = 0;
}

static unsigned uivector_push_back(uivector* p, unsigned c)
{
  if(!uivector_resize(p, p->size + 1)) return 0;
  p->data[p->size - 1] = c;
  return 1;
}

typedef struct ucvector
{
  unsigned char* data;
  size_t size;
  size_t allocsize;
} ucvector;


static unsigned ucvector_reserve(ucvector* p, size_t allocsize)
{
  if(allocsize > p->allocsize)
  {
    size_t newsize = (allocsize > p->allocsize * 2) ? allocsize : (allocsize * 3 / 2);
    void* data = realloc(p->data, newsize);
    if(data)
    {
      p->allocsize = newsize;
      p->data = (unsigned char*)data;
    }
    else return 0;
  }
  return 1;
}

static unsigned ucvector_resize(ucvector* p, size_t size)
{
  if(!ucvector_reserve(p, size * sizeof(unsigned char))) return 0;
  p->size = size;
  return 1;
}

static void ucvector_init_buffer(ucvector* p, unsigned char* buffer, size_t size)
{
  p->data = buffer;
  p->allocsize = p->size = size;
}

static unsigned ucvector_push_back(ucvector* p, unsigned char c)
{
  if(!ucvector_resize(p, p->size + 1)) return 0;
  p->data[p->size - 1] = c;
  return 1;
}

unsigned lodepng_read32bitInt(const unsigned char* buffer)
{
  return (unsigned)((buffer[0] << 24) | (buffer[1] << 16) | (buffer[2] << 8) | buffer[3]);
}

static void addBitsToStream(size_t* bitpointer, ucvector* bitstream, unsigned value, size_t nbits)
{
  size_t i;
  for(i = 0; i != nbits; ++i) { if(((*bitpointer) & 7) == 0) ucvector_push_back(bitstream, (unsigned char)0); (bitstream->data[bitstream->size - 1]) |= ((unsigned char)((value >> i) & 1) << ((*bitpointer) & 0x7)); ++(*bitpointer);};
}

static void addBitsToStreamReversed(size_t* bitpointer, ucvector* bitstream, unsigned value, size_t nbits)
{
  size_t i;
  for(i = 0; i != nbits; ++i) { if(((*bitpointer) & 7) == 0) ucvector_push_back(bitstream, (unsigned char)0); (bitstream->data[bitstream->size - 1]) |= ((unsigned char)((value >> (nbits - 1 - i)) & 1) << ((*bitpointer) & 0x7)); ++(*bitpointer);};
}

static unsigned char readBitFromStream(size_t* bitpointer, const unsigned char* bitstream)
{
  unsigned char result = (unsigned char)(((bitstream[*bitpointer >> 3] >> (*bitpointer & 0x7)) & (unsigned char)1));
  ++(*bitpointer);
  return result;
}

static unsigned readBitsFromStream(size_t* bitpointer, const unsigned char* bitstream, size_t nbits)
{
  unsigned result = 0, i;
  for(i = 0; i != nbits; ++i)
  {
    result += ((unsigned)((bitstream[*bitpointer >> 3] >> (*bitpointer & 0x7)) & (unsigned char)1)) << i;
    ++(*bitpointer);
  }
  return result;
}

static const unsigned LENGTHBASE[29]
  = {3, 4, 5, 6, 7, 8, 9, 10, 11, 13, 15, 17, 19, 23, 27, 31, 35, 43, 51, 59,
     67, 83, 99, 115, 131, 163, 195, 227, 258};

static const unsigned LENGTHEXTRA[29]
  = {0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 2, 2, 2, 2, 3, 3, 3, 3,
      4, 4, 4, 4, 5, 5, 5, 5, 0};

static const unsigned DISTANCEBASE[30]
  = {1, 2, 3, 4, 5, 7, 9, 13, 17, 25, 33, 49, 65, 97, 129, 193, 257, 385, 513,
     769, 1025, 1537, 2049, 3073, 4097, 6145, 8193, 12289, 16385, 24577};

static const unsigned DISTANCEEXTRA[30]
  = {0, 0, 0, 0, 1, 1, 2, 2, 3, 3, 4, 4, 5, 5, 6, 6, 7, 7, 8,
       8, 9, 9, 10, 10, 11, 11, 12, 12, 13, 13};

static const unsigned CLCL_ORDER[19]
  = {16, 17, 18, 0, 8, 7, 9, 6, 10, 5, 11, 4, 12, 3, 13, 2, 14, 1, 15};

typedef struct HuffmanTree
{
  unsigned* tree2d;
  unsigned* tree1d;
  unsigned* lengths;
  unsigned maxbitlen;
  unsigned numcodes;
} HuffmanTree;

static void HuffmanTree_init(HuffmanTree* tree)
{
  tree->tree2d = 0;
  tree->tree1d = 0;
  tree->lengths = 0;
}

static void HuffmanTree_cleanup(HuffmanTree* tree)
{
  free(tree->tree2d);
  free(tree->tree1d);
  free(tree->lengths);
}

static unsigned HuffmanTree_make2DTree(HuffmanTree* tree)
{
  unsigned nodefilled = 0;
  unsigned treepos = 0;
  unsigned n, i;

  tree->tree2d = (unsigned*)malloc(tree->numcodes * 2 * sizeof(unsigned));
  if(!tree->tree2d) return 83;

  for(n = 0; n < tree->numcodes * 2; ++n)
  {
    tree->tree2d[n] = 32767;
  }

  for(n = 0; n < tree->numcodes; ++n)
  {
    for(i = 0; i != tree->lengths[n]; ++i)
    {
      unsigned char bit = (unsigned char)((tree->tree1d[n] >> (tree->lengths[n] - i - 1)) & 1);

      if(treepos > 2147483647 || treepos + 2 > tree->numcodes) return 55;
      if(tree->tree2d[2 * treepos + bit] == 32767)
      {
        if(i + 1 == tree->lengths[n])
        {
          tree->tree2d[2 * treepos + bit] = n;
          treepos = 0;
        }
        else
        {
          ++nodefilled;
          tree->tree2d[2 * treepos + bit] = nodefilled + tree->numcodes;
          treepos = nodefilled;
        }
      }
      else treepos = tree->tree2d[2 * treepos + bit] - tree->numcodes;
    }
  }

  for(n = 0; n < tree->numcodes * 2; ++n)
  {
    if(tree->tree2d[n] == 32767) tree->tree2d[n] = 0;
  }

  return 0;
}

static unsigned HuffmanTree_makeFromLengths2(HuffmanTree* tree)
{
  uivector blcount;
  uivector nextcode;
  unsigned error = 0;
  unsigned bits, n;

  uivector_init(&blcount);
  uivector_init(&nextcode);

  tree->tree1d = (unsigned*)malloc(tree->numcodes * sizeof(unsigned));
  if(!tree->tree1d) error = 83;

  if(!uivector_resizev(&blcount, tree->maxbitlen + 1, 0)
  || !uivector_resizev(&nextcode, tree->maxbitlen + 1, 0))
    error = 83;

  if(!error)
  {

    for(bits = 0; bits != tree->numcodes; ++bits) ++blcount.data[tree->lengths[bits]];

    for(bits = 1; bits <= tree->maxbitlen; ++bits)
    {
      nextcode.data[bits] = (nextcode.data[bits - 1] + blcount.data[bits - 1]) << 1;
    }

    for(n = 0; n != tree->numcodes; ++n)
    {
      if(tree->lengths[n] != 0) tree->tree1d[n] = nextcode.data[tree->lengths[n]]++;
    }
  }

  uivector_cleanup(&blcount);
  uivector_cleanup(&nextcode);

  if(!error) return HuffmanTree_make2DTree(tree);
  else return error;
}

static unsigned HuffmanTree_makeFromLengths(HuffmanTree* tree, const unsigned* bitlen,
                                            size_t numcodes, unsigned maxbitlen)
{
  unsigned i;
  tree->lengths = (unsigned*)malloc(numcodes * sizeof(unsigned));
  if(!tree->lengths) return 83;
  for(i = 0; i != numcodes; ++i) tree->lengths[i] = bitlen[i];
  tree->numcodes = (unsigned)numcodes;
  tree->maxbitlen = maxbitlen;
  return HuffmanTree_makeFromLengths2(tree);
}

typedef struct BPMNode
{
  int weight;
  unsigned index;
  struct BPMNode* tail;
  int in_use;
} BPMNode;

typedef struct BPMLists
{

  unsigned memsize;
  BPMNode* memory;
  unsigned numfree;
  unsigned nextfree;
  BPMNode** freelist;

  unsigned listsize;
  BPMNode** chains0;
  BPMNode** chains1;
} BPMLists;

static BPMNode* bpmnode_create(BPMLists* lists, int weight, unsigned index, BPMNode* tail)
{
  unsigned i;
  BPMNode* result;

  if(lists->nextfree >= lists->numfree)
  {

    for(i = 0; i != lists->memsize; ++i) lists->memory[i].in_use = 0;
    for(i = 0; i != lists->listsize; ++i)
    {
      BPMNode* node;
      for(node = lists->chains0[i]; node != 0; node = node->tail) node->in_use = 1;
      for(node = lists->chains1[i]; node != 0; node = node->tail) node->in_use = 1;
    }

    lists->numfree = 0;
    for(i = 0; i != lists->memsize; ++i)
    {
      if(!lists->memory[i].in_use) lists->freelist[lists->numfree++] = &lists->memory[i];
    }
    lists->nextfree = 0;
  }

  result = lists->freelist[lists->nextfree++];
  result->weight = weight;
  result->index = index;
  result->tail = tail;
  return result;
}

static void bpmnode_sort(BPMNode* leaves, size_t num)
{
  BPMNode* mem = (BPMNode*)malloc(sizeof(*leaves) * num);
  size_t width, counter = 0;
  for(width = 1; width < num; width *= 2)
  {
    BPMNode* a = (counter & 1) ? mem : leaves;
    BPMNode* b = (counter & 1) ? leaves : mem;
    size_t p;
    for(p = 0; p < num; p += 2 * width)
    {
      size_t q = (p + width > num) ? num : (p + width);
      size_t r = (p + 2 * width > num) ? num : (p + 2 * width);
      size_t i = p, j = q, k;
      for(k = p; k < r; k++)
      {
        if(i < q && (j >= r || a[i].weight <= a[j].weight)) b[k] = a[i++];
        else b[k] = a[j++];
      }
    }
    counter++;
  }
  if(counter & 1) memcpy(leaves, mem, sizeof(*leaves) * num);
  free(mem);
}

static void boundaryPM(BPMLists* lists, BPMNode* leaves, size_t numpresent, int c, int num)
{
  unsigned lastindex = lists->chains1[c]->index;

  if(c == 0)
  {
    if(lastindex >= numpresent) return;
    lists->chains0[c] = lists->chains1[c];
    lists->chains1[c] = bpmnode_create(lists, leaves[lastindex].weight, lastindex + 1, 0);
  }
  else
  {

    int sum = lists->chains0[c - 1]->weight + lists->chains1[c - 1]->weight;
    lists->chains0[c] = lists->chains1[c];
    if(lastindex < numpresent && sum > leaves[lastindex].weight)
    {
      lists->chains1[c] = bpmnode_create(lists, leaves[lastindex].weight, lastindex + 1, lists->chains1[c]->tail);
      return;
    }
    lists->chains1[c] = bpmnode_create(lists, sum, lastindex, lists->chains1[c - 1]);


    if(num + 1 < (int)(2 * numpresent - 2))
    {
      boundaryPM(lists, leaves, numpresent, c - 1, num);
      boundaryPM(lists, leaves, numpresent, c - 1, num);
    }
  }
}

unsigned huffman_code_lengths(unsigned* lengths, const unsigned* frequencies,
                                      size_t numcodes, unsigned maxbitlen)
{
  unsigned error = 0;
  unsigned i;
  size_t numpresent = 0;
  BPMNode* leaves;

  if (numcodes == 0) return 80;
  if ((static_cast<size_t>(1u) << maxbitlen) < numcodes) return 80;

  leaves = (BPMNode*)malloc(numcodes * sizeof(*leaves));
  if(!leaves) return 83;

  for(i = 0; i != numcodes; ++i)
  {
    if(frequencies[i] > 0)
    {
      leaves[numpresent].weight = (int)frequencies[i];
      leaves[numpresent].index = i;
      ++numpresent;
    }
  }

  for(i = 0; i != numcodes; ++i) lengths[i] = 0;

  if(numpresent == 0)
  {
    lengths[0] = lengths[1] = 1;
  }
  else if(numpresent == 1)
  {
    lengths[leaves[0].index] = 1;
    lengths[leaves[0].index == 0 ? 1 : 0] = 1;
  }
  else
  {
    BPMLists lists;
    BPMNode* node;

    bpmnode_sort(leaves, numpresent);

    lists.listsize = maxbitlen;
    lists.memsize = 2 * maxbitlen * (maxbitlen + 1);
    lists.nextfree = 0;
    lists.numfree = lists.memsize;
    lists.memory = (BPMNode*)malloc(lists.memsize * sizeof(*lists.memory));
    lists.freelist = (BPMNode**)malloc(lists.memsize * sizeof(BPMNode*));
    lists.chains0 = (BPMNode**)malloc(lists.listsize * sizeof(BPMNode*));
    lists.chains1 = (BPMNode**)malloc(lists.listsize * sizeof(BPMNode*));
    if(!lists.memory || !lists.freelist || !lists.chains0 || !lists.chains1) error = 83;

    if(!error)
    {
      for(i = 0; i != lists.memsize; ++i) lists.freelist[i] = &lists.memory[i];

      bpmnode_create(&lists, leaves[0].weight, 1, 0);
      bpmnode_create(&lists, leaves[1].weight, 2, 0);

      for(i = 0; i != lists.listsize; ++i)
      {
        lists.chains0[i] = &lists.memory[0];
        lists.chains1[i] = &lists.memory[1];
      }


      for(i = 2; i != 2 * numpresent - 2; ++i) boundaryPM(&lists, leaves, numpresent, (int)maxbitlen - 1, (int)i);

      for(node = lists.chains1[maxbitlen - 1]; node; node = node->tail)
      {
        for(i = 0; i != node->index; ++i) ++lengths[leaves[i].index];
      }
    }

    free(lists.memory);
    free(lists.freelist);
    free(lists.chains0);
    free(lists.chains1);
  }

  free(leaves);
  return error;
}

static unsigned HuffmanTree_makeFromFrequencies(HuffmanTree* tree, const unsigned* frequencies,
                                                size_t mincodes, size_t numcodes, unsigned maxbitlen)
{
  unsigned error = 0;
  while(!frequencies[numcodes - 1] && numcodes > mincodes) --numcodes;
  tree->maxbitlen = maxbitlen;
  tree->numcodes = (unsigned)numcodes;
  tree->lengths = (unsigned*)realloc(tree->lengths, numcodes * sizeof(unsigned));
  if(!tree->lengths) return 83;

  memset(tree->lengths, 0, numcodes * sizeof(unsigned));

  error = huffman_code_lengths(tree->lengths, frequencies, numcodes, maxbitlen);
  if(!error) error = HuffmanTree_makeFromLengths2(tree);
  return error;
}

static unsigned HuffmanTree_getCode(const HuffmanTree* tree, unsigned index)
{
  return tree->tree1d[index];
}

static unsigned HuffmanTree_getLength(const HuffmanTree* tree, unsigned index)
{
  return tree->lengths[index];
}


static unsigned generateFixedLitLenTree(HuffmanTree* tree)
{
  unsigned i, error = 0;
  unsigned* bitlen = (unsigned*)malloc(288 * sizeof(unsigned));
  if(!bitlen) return 83;


  for(i = 0; i <= 143; ++i) bitlen[i] = 8;
  for(i = 144; i <= 255; ++i) bitlen[i] = 9;
  for(i = 256; i <= 279; ++i) bitlen[i] = 7;
  for(i = 280; i <= 287; ++i) bitlen[i] = 8;

  error = HuffmanTree_makeFromLengths(tree, bitlen, 288, 15);

  free(bitlen);
  return error;
}

static unsigned generateFixedDistanceTree(HuffmanTree* tree)
{
  unsigned i, error = 0;
  unsigned* bitlen = (unsigned*)malloc(32 * sizeof(unsigned));
  if(!bitlen) return 83;


  for(i = 0; i != 32; ++i) bitlen[i] = 5;
  error = HuffmanTree_makeFromLengths(tree, bitlen, 32, 15);

  free(bitlen);
  return error;
}

static unsigned huffmanDecodeSymbol(const unsigned char* in, size_t* bp,
                                    const HuffmanTree* codetree, size_t inbitlength)
{
  unsigned treepos = 0, ct;
  for(;;)
  {
    if(*bp >= inbitlength) return (unsigned)(-1);

    ct = codetree->tree2d[(treepos << 1) + ((in[*bp >> 3] >> (*bp & 0x7)) & (unsigned char)1)];
    ++(*bp);
    if(ct < codetree->numcodes) return ct;
    else treepos = ct - codetree->numcodes;

    if(treepos >= codetree->numcodes) return (unsigned)(-1);
  }
}

static void getTreeInflateFixed(HuffmanTree* tree_ll, HuffmanTree* tree_d)
{

  generateFixedLitLenTree(tree_ll);
  generateFixedDistanceTree(tree_d);
}

static unsigned getTreeInflateDynamic(HuffmanTree* tree_ll, HuffmanTree* tree_d,
                                      const unsigned char* in, size_t* bp, size_t inlength)
{

  unsigned error = 0;
  unsigned n, HLIT, HDIST, HCLEN, i;
  size_t inbitlength = inlength * 8;


  unsigned* bitlen_ll = 0;
  unsigned* bitlen_d = 0;

  unsigned* bitlen_cl = 0;
  HuffmanTree tree_cl;

  if((*bp) + 14 > (inlength << 3)) return 49;


  HLIT = readBitsFromStream(bp, in, 5) + 257;

  HDIST = readBitsFromStream(bp, in, 5) + 1;

  HCLEN = readBitsFromStream(bp, in, 4) + 4;

  if((*bp) + HCLEN * 3 > (inlength << 3)) return 50;

  HuffmanTree_init(&tree_cl);

  while(!error)
  {
    bitlen_cl = (unsigned*)malloc(19 * sizeof(unsigned));
    if(!bitlen_cl) { error = 83; break;};

    for(i = 0; i != 19; ++i)
    {
      if(i < HCLEN) bitlen_cl[CLCL_ORDER[i]] = readBitsFromStream(bp, in, 3);
      else bitlen_cl[CLCL_ORDER[i]] = 0;
    }

    error = HuffmanTree_makeFromLengths(&tree_cl, bitlen_cl, 19, 7);
    if (error) break;

    bitlen_ll = (unsigned*)malloc(288 * sizeof(unsigned));
    bitlen_d = (unsigned*)malloc(32 * sizeof(unsigned));
    if(!bitlen_ll || !bitlen_d) { error = 83; break;};
    for(i = 0; i != 288; ++i) bitlen_ll[i] = 0;
    for(i = 0; i != 32; ++i) bitlen_d[i] = 0;

    i = 0;
    while(i < HLIT + HDIST)
    {
      unsigned code = huffmanDecodeSymbol(in, bp, &tree_cl, inbitlength);
      if(code <= 15)
      {
        if(i < HLIT) bitlen_ll[i] = code;
        else bitlen_d[i - HLIT] = code;
        ++i;
      }
      else if(code == 16)
      {
        unsigned replength = 3;
        unsigned value;

        if(i == 0) { error = 54; break;};

        if((*bp + 2) > inbitlength) { error = 50; break;};
        replength += readBitsFromStream(bp, in, 2);

        if(i < HLIT + 1) value = bitlen_ll[i - 1];
        else value = bitlen_d[i - HLIT - 1];

        for(n = 0; n < replength; ++n)
        {
          if(i >= HLIT + HDIST) { error = 13; break;};
          if(i < HLIT) bitlen_ll[i] = value;
          else bitlen_d[i - HLIT] = value;
          ++i;
        }
      }
      else if(code == 17)
      {
        unsigned replength = 3;
        if((*bp + 3) > inbitlength) { error = 50; break;};
        replength += readBitsFromStream(bp, in, 3);


        for(n = 0; n < replength; ++n)
        {
          if(i >= HLIT + HDIST) { error = 14; break;};

          if(i < HLIT) bitlen_ll[i] = 0;
          else bitlen_d[i - HLIT] = 0;
          ++i;
        }
      }
      else if(code == 18)
      {
        unsigned replength = 11;
        if((*bp + 7) > inbitlength) { error = 50; break;};
        replength += readBitsFromStream(bp, in, 7);


        for(n = 0; n < replength; ++n)
        {
          if(i >= HLIT + HDIST) { error = 15; break;};

          if(i < HLIT) bitlen_ll[i] = 0;
          else bitlen_d[i - HLIT] = 0;
          ++i;
        }
      }
      else
      {
        if(code == (unsigned)(-1))
        {
          error = (*bp) > inbitlength ? 10 : 11;
        }
        else error = 16;
        break;
      }
    }
    if(error) break;

    if(bitlen_ll[256] == 0) { error = 64; break;};

    error = HuffmanTree_makeFromLengths(tree_ll, bitlen_ll, 288, 15);
    if(error) break;
    error = HuffmanTree_makeFromLengths(tree_d, bitlen_d, 32, 15);

    break;
  }

  free(bitlen_cl);
  free(bitlen_ll);
  free(bitlen_d);
  HuffmanTree_cleanup(&tree_cl);

  return error;
}

static unsigned inflateHuffmanBlock(ucvector* out, const unsigned char* in, size_t* bp,
                                    size_t* pos, size_t inlength, unsigned btype)
{
  unsigned error = 0;
  HuffmanTree tree_ll;
  HuffmanTree tree_d;
  size_t inbitlength = inlength * 8;

  HuffmanTree_init(&tree_ll);
  HuffmanTree_init(&tree_d);

  if(btype == 1) getTreeInflateFixed(&tree_ll, &tree_d);
  else if(btype == 2) error = getTreeInflateDynamic(&tree_ll, &tree_d, in, bp, inlength);

  while(!error)
  {

    unsigned code_ll = huffmanDecodeSymbol(in, bp, &tree_ll, inbitlength);
    if(code_ll <= 255)
    {
      if(!ucvector_resize(out, (*pos) + 1)) { error = 83; break;};
      out->data[*pos] = (unsigned char)code_ll;
      ++(*pos);
    }
    else if(code_ll >= 257 && code_ll <= 285)
    {
      unsigned code_d, distance;
      unsigned numextrabits_l, numextrabits_d;
      size_t start, forward, backward, length;

      length = LENGTHBASE[code_ll - 257];

      numextrabits_l = LENGTHEXTRA[code_ll - 257];
      if((*bp + numextrabits_l) > inbitlength) { error = 51; break;};
      length += readBitsFromStream(bp, in, numextrabits_l);

      code_d = huffmanDecodeSymbol(in, bp, &tree_d, inbitlength);
      if(code_d > 29)
      {
        if(code_ll == (unsigned)(-1))
        {
          error = (*bp) > inlength * 8 ? 10 : 11;
        }
        else error = 18;
        break;
      }
      distance = DISTANCEBASE[code_d];
      numextrabits_d = DISTANCEEXTRA[code_d];
      if((*bp + numextrabits_d) > inbitlength) { error = 51; break;};
      distance += readBitsFromStream(bp, in, numextrabits_d);

      start = (*pos);
      if(distance > start) { error = 52; break;};
      backward = start - distance;

      if(!ucvector_resize(out, (*pos) + length)) { error = 83; break;};
      if (distance < length) {
        for(forward = 0; forward < length; ++forward)
        {
          out->data[(*pos)++] = out->data[backward++];
        }
      } else {
        memcpy(out->data + *pos, out->data + backward, length);
        *pos += length;
      }
    }
    else if(code_ll == 256)
    {
      break;
    }
    else
    {
      error = ((*bp) > inlength * 8) ? 10 : 11;
      break;
    }
  }

  HuffmanTree_cleanup(&tree_ll);
  HuffmanTree_cleanup(&tree_d);

  return error;
}

static unsigned inflateNoCompression(ucvector* out, const unsigned char* in, size_t* bp, size_t* pos, size_t inlength)
{
  size_t p;
  unsigned LEN, NLEN, n, error = 0;


  while(((*bp) & 0x7) != 0) ++(*bp);
  p = (*bp) / 8;


  if(p + 4 >= inlength) return 52;
  LEN = in[p] + 256u * in[p + 1]; p += 2;
  NLEN = in[p] + 256u * in[p + 1]; p += 2;


  if(LEN + NLEN != 65535) return 21;

  if(!ucvector_resize(out, (*pos) + LEN)) return 83;


  if(p + LEN > inlength) return 23;
  for(n = 0; n < LEN; ++n) out->data[(*pos)++] = in[p++];

  (*bp) = p * 8;

  return error;
}

static unsigned inflatev(ucvector* out,
                                 const unsigned char* in, size_t insize)
{
  size_t bp = 0;
  unsigned BFINAL = 0;
  size_t pos = 0;
  unsigned error = 0;

  while(!BFINAL)
  {
    unsigned BTYPE;
    if(bp + 2 >= insize * 8) return 52;
    BFINAL = readBitFromStream(&bp, in);
    BTYPE = 1u * readBitFromStream(&bp, in);
    BTYPE += 2u * readBitFromStream(&bp, in);

    if(BTYPE == 3) return 20;
    else if(BTYPE == 0) error = inflateNoCompression(out, in, &bp, &pos, insize);
    else error = inflateHuffmanBlock(out, in, &bp, &pos, insize, BTYPE);

    if(error) return error;
  }

  return error;
}

unsigned file::inflate(unsigned char** out, size_t* outsize,
                         const unsigned char* in, size_t insize)
{
  unsigned error;
  ucvector v;
  ucvector_init_buffer(&v, *out, *outsize);
  error = inflatev(&v, in, insize);
  *out = v.data;
  *outsize = v.size;
  return error;
}

static const size_t MAX_SUPPORTED_DEFLATE_LENGTH = 258;

static void addHuffmanSymbol(size_t* bp, ucvector* compressed, unsigned code, unsigned bitlen)
{
  addBitsToStreamReversed(bp, compressed, code, bitlen);
}

static size_t searchCodeIndex(const unsigned* array, size_t array_size, size_t value)
{
  size_t left = 1;
  size_t right = array_size - 1;

  while(left <= right) {
    size_t mid = (left + right) >> 1;
    if (array[mid] >= value) right = mid - 1;
    else left = mid + 1;
  }
  if(left >= array_size || array[left] > value) left--;
  return left;
}

static void addLengthDistance(uivector* values, size_t length, size_t distance)
{
  unsigned length_code = (unsigned)searchCodeIndex(LENGTHBASE, 29, length);
  unsigned extra_length = (unsigned)(length - LENGTHBASE[length_code]);
  unsigned dist_code = (unsigned)searchCodeIndex(DISTANCEBASE, 30, distance);
  unsigned extra_distance = (unsigned)(distance - DISTANCEBASE[dist_code]);

  uivector_push_back(values, length_code + 257);
  uivector_push_back(values, extra_length);
  uivector_push_back(values, dist_code);
  uivector_push_back(values, extra_distance);
}

static const unsigned HASH_NUM_VALUES = 65536;
static const unsigned HASH_BIT_MASK = 65535;

typedef struct Hash
{
  int* head;

  unsigned short* chain;
  int* val;
  int* headz;
  unsigned short* chainz;
  unsigned short* zeros;
} Hash;

static unsigned hash_init(Hash* hash, unsigned windowsize)
{
  unsigned i;
  hash->head = (int*)malloc(sizeof(int) * HASH_NUM_VALUES);
  hash->val = (int*)malloc(sizeof(int) * windowsize);
  hash->chain = (unsigned short*)malloc(sizeof(unsigned short) * windowsize);

  hash->zeros = (unsigned short*)malloc(sizeof(unsigned short) * windowsize);
  hash->headz = (int*)malloc(sizeof(int) * (MAX_SUPPORTED_DEFLATE_LENGTH + 1));
  hash->chainz = (unsigned short*)malloc(sizeof(unsigned short) * windowsize);

  if(!hash->head || !hash->chain || !hash->val || !hash->headz|| !hash->chainz || !hash->zeros)
  {
    return 83;
  }

  for(i = 0; i != HASH_NUM_VALUES; ++i) hash->head[i] = -1;
  for(i = 0; i != windowsize; ++i) hash->val[i] = -1;
  for(i = 0; i != windowsize; ++i) hash->chain[i] = i;

  for(i = 0; i <= MAX_SUPPORTED_DEFLATE_LENGTH; ++i) hash->headz[i] = -1;
  for(i = 0; i != windowsize; ++i) hash->chainz[i] = i;

  return 0;
}

static void hash_cleanup(Hash* hash)
{
  free(hash->head);
  free(hash->val);
  free(hash->chain);

  free(hash->zeros);
  free(hash->headz);
  free(hash->chainz);
}

static unsigned getHash(const unsigned char* data, size_t size, size_t pos)
{
  unsigned result = 0;
  if(pos + 2 < size)
  {
    result ^= (unsigned)(data[pos + 0] << 0u);
    result ^= (unsigned)(data[pos + 1] << 4u);
    result ^= (unsigned)(data[pos + 2] << 8u);
  } else {
    size_t amount, i;
    if(pos >= size) return 0;
    amount = size - pos;
    for(i = 0; i != amount; ++i) result ^= (unsigned)(data[pos + i] << (i * 8u));
  }
  return result & HASH_BIT_MASK;
}

static unsigned countZeros(const unsigned char* data, size_t size, size_t pos)
{
  const unsigned char* start = data + pos;
  const unsigned char* end = start + MAX_SUPPORTED_DEFLATE_LENGTH;
  if(end > data + size) end = data + size;
  data = start;
  while(data != end && *data == 0) ++data;

  return (unsigned)(data - start);
}

static void updateHashChain(Hash* hash, unsigned wpos, unsigned hashval, unsigned short numzeros)
{
  hash->val[wpos] = (int)hashval;
  if(hash->head[hashval] != -1) hash->chain[wpos] = hash->head[hashval];
  hash->head[hashval] = wpos;

  hash->zeros[wpos] = numzeros;
  if(hash->headz[numzeros] != -1) hash->chainz[wpos] = hash->headz[numzeros];
  hash->headz[numzeros] = wpos;
}

static unsigned encodeLZ77(uivector* out, Hash* hash,
                           const unsigned char* in, size_t inpos, size_t insize, unsigned windowsize,
                           unsigned minmatch, unsigned nicematch, unsigned lazymatching)
{
  size_t pos;
  unsigned i, error = 0;

  unsigned maxchainlength = windowsize >= 8192 ? windowsize : windowsize / 8;
  unsigned maxlazymatch = windowsize >= 8192 ? MAX_SUPPORTED_DEFLATE_LENGTH : 64;

  unsigned usezeros = 1;
  unsigned numzeros = 0;

  unsigned offset;
  unsigned length;
  unsigned lazy = 0;
  unsigned lazylength = 0, lazyoffset = 0;
  unsigned hashval;
  unsigned current_offset, current_length;
  unsigned prev_offset;
  const unsigned char *lastptr, *foreptr, *backptr;
  unsigned hashpos;

  if(windowsize == 0 || windowsize > 32768) return 60;
  if((windowsize & (windowsize - 1)) != 0) return 90;

  if(nicematch > MAX_SUPPORTED_DEFLATE_LENGTH) nicematch = MAX_SUPPORTED_DEFLATE_LENGTH;

  for(pos = inpos; pos < insize; ++pos)
  {
    unsigned wpos = pos & (windowsize - 1);
    unsigned chainlength = 0;

    hashval = getHash(in, insize, pos);

    if(usezeros && hashval == 0)
    {
      if(numzeros == 0) numzeros = countZeros(in, insize, pos);
      else if(pos + numzeros > insize || in[pos + numzeros - 1] != 0) --numzeros;
    }
    else
    {
      numzeros = 0;
    }

    updateHashChain(hash, wpos, hashval, numzeros);

    length = 0;
    offset = 0;

    hashpos = hash->chain[wpos];

    lastptr = &in[insize < pos + MAX_SUPPORTED_DEFLATE_LENGTH ? insize : pos + MAX_SUPPORTED_DEFLATE_LENGTH];

    prev_offset = 0;
    for(;;)
    {
      if(chainlength++ >= maxchainlength) break;
      current_offset = hashpos <= wpos ? wpos - hashpos : wpos - hashpos + windowsize;

      if(current_offset < prev_offset) break;
      prev_offset = current_offset;
      if(current_offset > 0)
      {
        foreptr = &in[pos];
        backptr = &in[pos - current_offset];

        if(numzeros >= 3)
        {
          unsigned skip = hash->zeros[hashpos];
          if(skip > numzeros) skip = numzeros;
          backptr += skip;
          foreptr += skip;
        }

        while(foreptr != lastptr && *backptr == *foreptr)
        {
          ++backptr;
          ++foreptr;
        }
        current_length = (unsigned)(foreptr - &in[pos]);

        if(current_length > length)
        {
          length = current_length;
          offset = current_offset;
          if(current_length >= nicematch) break;
        }
      }

      if(hashpos == hash->chain[hashpos]) break;

      if(numzeros >= 3 && length > numzeros)
      {
        hashpos = hash->chainz[hashpos];
        if(hash->zeros[hashpos] != numzeros) break;
      }
      else
      {
        hashpos = hash->chain[hashpos];

        if(hash->val[hashpos] != (int)hashval) break;
      }
    }

    if(lazymatching)
    {
      if(!lazy && length >= 3 && length <= maxlazymatch && length < MAX_SUPPORTED_DEFLATE_LENGTH)
      {
        lazy = 1;
        lazylength = length;
        lazyoffset = offset;
        continue;
      }
      if(lazy)
      {
        lazy = 0;
        if(pos == 0) { error = 81; break;};
        if(length > lazylength + 1)
        {

          if(!uivector_push_back(out, in[pos - 1])) { error = 83; break;};
        }
        else
        {
          length = lazylength;
          offset = lazyoffset;
          hash->head[hashval] = -1;
          hash->headz[numzeros] = -1;
          --pos;
        }
      }
    }
    if(length >= 3 && offset > windowsize) { error = 86; break;};

    if(length < 3)
    {
      if(!uivector_push_back(out, in[pos])) { error = 83; break;};
    }
    else if(length < minmatch || (length == 3 && offset > 4096))
    {
      if(!uivector_push_back(out, in[pos])) { error = 83; break;};
    }
    else
    {
      addLengthDistance(out, length, offset);
      for(i = 1; i < length; ++i)
      {
        ++pos;
        wpos = pos & (windowsize - 1);
        hashval = getHash(in, insize, pos);
        if(usezeros && hashval == 0)
        {
          if(numzeros == 0) numzeros = countZeros(in, insize, pos);
          else if(pos + numzeros > insize || in[pos + numzeros - 1] != 0) --numzeros;
        }
        else
        {
          numzeros = 0;
        }
        updateHashChain(hash, wpos, hashval, numzeros);
      }
    }
  }

  return error;
}

static unsigned deflateNoCompression(ucvector* out, const unsigned char* data, size_t datasize)
{
  size_t i, j, numdeflateblocks = (datasize + 65534) / 65535;
  unsigned datapos = 0;
  for(i = 0; i != numdeflateblocks; ++i)
  {
    unsigned BFINAL, BTYPE, LEN, NLEN;
    unsigned char firstbyte;

    BFINAL = (i == numdeflateblocks - 1);
    BTYPE = 0;

    firstbyte = (unsigned char)(BFINAL + ((BTYPE & 1) << 1) + ((BTYPE & 2) << 1));
    ucvector_push_back(out, firstbyte);

    LEN = 65535;
    if(datasize - datapos < 65535) LEN = (unsigned)datasize - datapos;
    NLEN = 65535 - LEN;

    ucvector_push_back(out, (unsigned char)(LEN & 255));
    ucvector_push_back(out, (unsigned char)(LEN >> 8));
    ucvector_push_back(out, (unsigned char)(NLEN & 255));
    ucvector_push_back(out, (unsigned char)(NLEN >> 8));

    for(j = 0; j < 65535 && datapos < datasize; ++j)
    {
      ucvector_push_back(out, data[datapos++]);
    }
  }

  return 0;
}

static void writeLZ77data(size_t* bp, ucvector* out, const uivector* lz77_encoded,
                          const HuffmanTree* tree_ll, const HuffmanTree* tree_d)
{
  size_t i = 0;
  for(i = 0; i != lz77_encoded->size; ++i)
  {
    unsigned val = lz77_encoded->data[i];
    addHuffmanSymbol(bp, out, HuffmanTree_getCode(tree_ll, val), HuffmanTree_getLength(tree_ll, val));
    if(val > 256)
    {
      unsigned length_index = val - 257;
      unsigned n_length_extra_bits = LENGTHEXTRA[length_index];
      unsigned length_extra_bits = lz77_encoded->data[++i];

      unsigned distance_code = lz77_encoded->data[++i];

      unsigned distance_index = distance_code;
      unsigned n_distance_extra_bits = DISTANCEEXTRA[distance_index];
      unsigned distance_extra_bits = lz77_encoded->data[++i];

      addBitsToStream(bp, out, length_extra_bits, n_length_extra_bits);
      addHuffmanSymbol(bp, out, HuffmanTree_getCode(tree_d, distance_code),
                       HuffmanTree_getLength(tree_d, distance_code));
      addBitsToStream(bp, out, distance_extra_bits, n_distance_extra_bits);
    }
  }
}

static unsigned deflateDynamic(ucvector* out, size_t* bp, Hash* hash,
                               const unsigned char* data, size_t datapos, size_t dataend,
                               const file::LodePNGCompressSettings* settings, unsigned final)
{
  unsigned error = 0;

  uivector lz77_encoded;
  HuffmanTree tree_ll;
  HuffmanTree tree_d;
  HuffmanTree tree_cl;
  uivector frequencies_ll;
  uivector frequencies_d;
  uivector frequencies_cl;
  uivector bitlen_lld;
  uivector bitlen_lld_e;

  uivector bitlen_cl;
  size_t datasize = dataend - datapos;

  unsigned BFINAL = final;
  size_t numcodes_ll, numcodes_d, i;
  unsigned HLIT, HDIST, HCLEN;

  uivector_init(&lz77_encoded);
  HuffmanTree_init(&tree_ll);
  HuffmanTree_init(&tree_d);
  HuffmanTree_init(&tree_cl);
  uivector_init(&frequencies_ll);
  uivector_init(&frequencies_d);
  uivector_init(&frequencies_cl);
  uivector_init(&bitlen_lld);
  uivector_init(&bitlen_lld_e);
  uivector_init(&bitlen_cl);

  while(!error)
  {
    if(settings->use_lz77)
    {
      error = encodeLZ77(&lz77_encoded, hash, data, datapos, dataend, settings->windowsize,
                         settings->minmatch, settings->nicematch, settings->lazymatching);
      if(error) break;
    }
    else
    {
      if(!uivector_resize(&lz77_encoded, datasize)) { error = 83; break;};
      for(i = datapos; i < dataend; ++i) lz77_encoded.data[i - datapos] = data[i];
    }

    if(!uivector_resizev(&frequencies_ll, 286, 0)) { error = 83; break;};
    if(!uivector_resizev(&frequencies_d, 30, 0)) { error = 83; break;};

    for(i = 0; i != lz77_encoded.size; ++i)
    {
      unsigned symbol = lz77_encoded.data[i];
      ++frequencies_ll.data[symbol];
      if(symbol > 256)
      {
        unsigned dist = lz77_encoded.data[i + 2];
        ++frequencies_d.data[dist];
        i += 3;
      }
    }
    frequencies_ll.data[256] = 1;

    error = HuffmanTree_makeFromFrequencies(&tree_ll, frequencies_ll.data, 257, frequencies_ll.size, 15);
    if(error) break;

    error = HuffmanTree_makeFromFrequencies(&tree_d, frequencies_d.data, 2, frequencies_d.size, 15);
    if(error) break;

    numcodes_ll = tree_ll.numcodes; if(numcodes_ll > 286) numcodes_ll = 286;
    numcodes_d = tree_d.numcodes; if(numcodes_d > 30) numcodes_d = 30;

    for(i = 0; i != numcodes_ll; ++i) uivector_push_back(&bitlen_lld, HuffmanTree_getLength(&tree_ll, (unsigned)i));
    for(i = 0; i != numcodes_d; ++i) uivector_push_back(&bitlen_lld, HuffmanTree_getLength(&tree_d, (unsigned)i));

    for(i = 0; i != (unsigned)bitlen_lld.size; ++i)
    {
      unsigned j = 0;
      while(i + j + 1 < (unsigned)bitlen_lld.size && bitlen_lld.data[i + j + 1] == bitlen_lld.data[i]) ++j;

      if(bitlen_lld.data[i] == 0 && j >= 2)
      {
        ++j;
        if(j <= 10)
        {
          uivector_push_back(&bitlen_lld_e, 17);
          uivector_push_back(&bitlen_lld_e, j - 3);
        }
        else
        {
          if(j > 138) j = 138;
          uivector_push_back(&bitlen_lld_e, 18);
          uivector_push_back(&bitlen_lld_e, j - 11);
        }
        i += (j - 1);
      }
      else if(j >= 3)
      {
        size_t k;
        unsigned num = j / 6, rest = j % 6;
        uivector_push_back(&bitlen_lld_e, bitlen_lld.data[i]);
        for(k = 0; k < num; ++k)
        {
          uivector_push_back(&bitlen_lld_e, 16);
          uivector_push_back(&bitlen_lld_e, 6 - 3);
        }
        if(rest >= 3)
        {
          uivector_push_back(&bitlen_lld_e, 16);
          uivector_push_back(&bitlen_lld_e, rest - 3);
        }
        else j -= rest;
        i += j;
      }
      else
      {
        uivector_push_back(&bitlen_lld_e, bitlen_lld.data[i]);
      }
    }

    if(!uivector_resizev(&frequencies_cl, 19, 0)) { error = 83; break;};
    for(i = 0; i != bitlen_lld_e.size; ++i)
    {
      ++frequencies_cl.data[bitlen_lld_e.data[i]];
      if(bitlen_lld_e.data[i] >= 16) ++i;
    }

    error = HuffmanTree_makeFromFrequencies(&tree_cl, frequencies_cl.data,
                                            frequencies_cl.size, frequencies_cl.size, 7);
    if(error) break;

    if(!uivector_resize(&bitlen_cl, tree_cl.numcodes)) { error = 83; break;};
    for(i = 0; i != tree_cl.numcodes; ++i)
    {
      bitlen_cl.data[i] = HuffmanTree_getLength(&tree_cl, CLCL_ORDER[i]);
    }
    while(bitlen_cl.data[bitlen_cl.size - 1] == 0 && bitlen_cl.size > 4)
    {
      if(!uivector_resize(&bitlen_cl, bitlen_cl.size - 1)) { error = 83; break;};
    }
    if(error) break;

    { if(((*bp) & 7) == 0) ucvector_push_back(out, (unsigned char)0); (out->data[out->size - 1]) |= (BFINAL << ((*bp) & 0x7)); ++(*bp);};
    { if(((*bp) & 7) == 0) ucvector_push_back(out, (unsigned char)0); (out->data[out->size - 1]) |= (0 << ((*bp) & 0x7)); ++(*bp);};
    { if(((*bp) & 7) == 0) ucvector_push_back(out, (unsigned char)0); (out->data[out->size - 1]) |= (1 << ((*bp) & 0x7)); ++(*bp);};

    HLIT = (unsigned)(numcodes_ll - 257);
    HDIST = (unsigned)(numcodes_d - 1);
    HCLEN = (unsigned)bitlen_cl.size - 4;

    while(!bitlen_cl.data[HCLEN + 4 - 1] && HCLEN > 0) --HCLEN;
    addBitsToStream(bp, out, HLIT, 5);
    addBitsToStream(bp, out, HDIST, 5);
    addBitsToStream(bp, out, HCLEN, 4);

    for(i = 0; i != HCLEN + 4; ++i) addBitsToStream(bp, out, bitlen_cl.data[i], 3);

    for(i = 0; i != bitlen_lld_e.size; ++i)
    {
      addHuffmanSymbol(bp, out, HuffmanTree_getCode(&tree_cl, bitlen_lld_e.data[i]),
                       HuffmanTree_getLength(&tree_cl, bitlen_lld_e.data[i]));

      if(bitlen_lld_e.data[i] == 16) addBitsToStream(bp, out, bitlen_lld_e.data[++i], 2);
      else if(bitlen_lld_e.data[i] == 17) addBitsToStream(bp, out, bitlen_lld_e.data[++i], 3);
      else if(bitlen_lld_e.data[i] == 18) addBitsToStream(bp, out, bitlen_lld_e.data[++i], 7);
    }

    writeLZ77data(bp, out, &lz77_encoded, &tree_ll, &tree_d);

    if(HuffmanTree_getLength(&tree_ll, 256) == 0) { error = 64; break;};

    addHuffmanSymbol(bp, out, HuffmanTree_getCode(&tree_ll, 256), HuffmanTree_getLength(&tree_ll, 256));
    break;
  }

  uivector_cleanup(&lz77_encoded);
  HuffmanTree_cleanup(&tree_ll);
  HuffmanTree_cleanup(&tree_d);
  HuffmanTree_cleanup(&tree_cl);
  uivector_cleanup(&frequencies_ll);
  uivector_cleanup(&frequencies_d);
  uivector_cleanup(&frequencies_cl);
  uivector_cleanup(&bitlen_lld_e);
  uivector_cleanup(&bitlen_lld);
  uivector_cleanup(&bitlen_cl);

  return error;
}

static unsigned deflateFixed(ucvector* out, size_t* bp, Hash* hash,
                             const unsigned char* data,
                             size_t datapos, size_t dataend,
                             const file::LodePNGCompressSettings* settings, unsigned final)
{
  HuffmanTree tree_ll;
  HuffmanTree tree_d;

  unsigned BFINAL = final;
  unsigned error = 0;
  size_t i;

  HuffmanTree_init(&tree_ll);
  HuffmanTree_init(&tree_d);

  generateFixedLitLenTree(&tree_ll);
  generateFixedDistanceTree(&tree_d);

  { if(((*bp) & 7) == 0) ucvector_push_back(out, (unsigned char)0); (out->data[out->size - 1]) |= (BFINAL << ((*bp) & 0x7)); ++(*bp);};
  { if(((*bp) & 7) == 0) ucvector_push_back(out, (unsigned char)0); (out->data[out->size - 1]) |= (1 << ((*bp) & 0x7)); ++(*bp);};
  { if(((*bp) & 7) == 0) ucvector_push_back(out, (unsigned char)0); (out->data[out->size - 1]) |= (0 << ((*bp) & 0x7)); ++(*bp);};

  if(settings->use_lz77)
  {
    uivector lz77_encoded;
    uivector_init(&lz77_encoded);
    error = encodeLZ77(&lz77_encoded, hash, data, datapos, dataend, settings->windowsize,
                       settings->minmatch, settings->nicematch, settings->lazymatching);
    if(!error) writeLZ77data(bp, out, &lz77_encoded, &tree_ll, &tree_d);
    uivector_cleanup(&lz77_encoded);
  }
  else
  {
    for(i = datapos; i < dataend; ++i)
    {
      addHuffmanSymbol(bp, out, HuffmanTree_getCode(&tree_ll, data[i]), HuffmanTree_getLength(&tree_ll, data[i]));
    }
  }

  if(!error) addHuffmanSymbol(bp, out, HuffmanTree_getCode(&tree_ll, 256), HuffmanTree_getLength(&tree_ll, 256));

  HuffmanTree_cleanup(&tree_ll);
  HuffmanTree_cleanup(&tree_d);

  return error;
}

static unsigned deflatev(ucvector* out, const unsigned char* in, size_t insize,
                                 const file::LodePNGCompressSettings* settings)
{
  unsigned error = 0;
  size_t i, blocksize, numdeflateblocks;
  size_t bp = 0;
  Hash hash;

  if(settings->btype > 2) return 61;
  else if(settings->btype == 0) return deflateNoCompression(out, in, insize);
  else if(settings->btype == 1) blocksize = insize;
  else
  {

    blocksize = insize / 8 + 8;
    if(blocksize < 65536) blocksize = 65536;
    if(blocksize > 262144) blocksize = 262144;
  }

  numdeflateblocks = (insize + blocksize - 1) / blocksize;
  if(numdeflateblocks == 0) numdeflateblocks = 1;

  error = hash_init(&hash, settings->windowsize);
  if(error) return error;

  for(i = 0; i != numdeflateblocks && !error; ++i)
  {
    unsigned final = (i == numdeflateblocks - 1);
    size_t start = i * blocksize;
    size_t end = start + blocksize;
    if(end > insize) end = insize;

    if(settings->btype == 1) error = deflateFixed(out, &bp, &hash, in, start, end, settings, final);
    else if(settings->btype == 2) error = deflateDynamic(out, &bp, &hash, in, start, end, settings, final);
  }

  hash_cleanup(&hash);

  return error;
}

unsigned file::deflate(unsigned char** out, size_t* outsize,
                         const unsigned char* in, size_t insize)
{
  unsigned error;
  ucvector v;
  ucvector_init_buffer(&v, *out, *outsize);
  error = deflatev(&v, in, insize, &lodepng_default_compress_settings);
  *out = v.data;
  *outsize = v.size;
  return error;
}

void file::compress_settings_init(LodePNGCompressSettings* settings)
{

  settings->btype = 2;
  settings->use_lz77 = 1;
  settings->windowsize = 2048;
  settings->minmatch = 3;
  settings->nicematch = 128;
  settings->lazymatching = 1;
}

const file::LodePNGCompressSettings file::lodepng_default_compress_settings = {2, 1, 2048, 3, 128, 1};

const uint32_t crc_table[256] =
{
  0x00000000UL, 0x77073096UL, 0xee0e612cUL, 0x990951baUL, 0x076dc419UL,
  0x706af48fUL, 0xe963a535UL, 0x9e6495a3UL, 0x0edb8832UL, 0x79dcb8a4UL,
  0xe0d5e91eUL, 0x97d2d988UL, 0x09b64c2bUL, 0x7eb17cbdUL, 0xe7b82d07UL,
  0x90bf1d91UL, 0x1db71064UL, 0x6ab020f2UL, 0xf3b97148UL, 0x84be41deUL,
  0x1adad47dUL, 0x6ddde4ebUL, 0xf4d4b551UL, 0x83d385c7UL, 0x136c9856UL,
  0x646ba8c0UL, 0xfd62f97aUL, 0x8a65c9ecUL, 0x14015c4fUL, 0x63066cd9UL,
  0xfa0f3d63UL, 0x8d080df5UL, 0x3b6e20c8UL, 0x4c69105eUL, 0xd56041e4UL,
  0xa2677172UL, 0x3c03e4d1UL, 0x4b04d447UL, 0xd20d85fdUL, 0xa50ab56bUL,
  0x35b5a8faUL, 0x42b2986cUL, 0xdbbbc9d6UL, 0xacbcf940UL, 0x32d86ce3UL,
  0x45df5c75UL, 0xdcd60dcfUL, 0xabd13d59UL, 0x26d930acUL, 0x51de003aUL,
  0xc8d75180UL, 0xbfd06116UL, 0x21b4f4b5UL, 0x56b3c423UL, 0xcfba9599UL,
  0xb8bda50fUL, 0x2802b89eUL, 0x5f058808UL, 0xc60cd9b2UL, 0xb10be924UL,
  0x2f6f7c87UL, 0x58684c11UL, 0xc1611dabUL, 0xb6662d3dUL, 0x76dc4190UL,
  0x01db7106UL, 0x98d220bcUL, 0xefd5102aUL, 0x71b18589UL, 0x06b6b51fUL,
  0x9fbfe4a5UL, 0xe8b8d433UL, 0x7807c9a2UL, 0x0f00f934UL, 0x9609a88eUL,
  0xe10e9818UL, 0x7f6a0dbbUL, 0x086d3d2dUL, 0x91646c97UL, 0xe6635c01UL,
  0x6b6b51f4UL, 0x1c6c6162UL, 0x856530d8UL, 0xf262004eUL, 0x6c0695edUL,
  0x1b01a57bUL, 0x8208f4c1UL, 0xf50fc457UL, 0x65b0d9c6UL, 0x12b7e950UL,
  0x8bbeb8eaUL, 0xfcb9887cUL, 0x62dd1ddfUL, 0x15da2d49UL, 0x8cd37cf3UL,
  0xfbd44c65UL, 0x4db26158UL, 0x3ab551ceUL, 0xa3bc0074UL, 0xd4bb30e2UL,
  0x4adfa541UL, 0x3dd895d7UL, 0xa4d1c46dUL, 0xd3d6f4fbUL, 0x4369e96aUL,
  0x346ed9fcUL, 0xad678846UL, 0xda60b8d0UL, 0x44042d73UL, 0x33031de5UL,
  0xaa0a4c5fUL, 0xdd0d7cc9UL, 0x5005713cUL, 0x270241aaUL, 0xbe0b1010UL,
  0xc90c2086UL, 0x5768b525UL, 0x206f85b3UL, 0xb966d409UL, 0xce61e49fUL,
  0x5edef90eUL, 0x29d9c998UL, 0xb0d09822UL, 0xc7d7a8b4UL, 0x59b33d17UL,
  0x2eb40d81UL, 0xb7bd5c3bUL, 0xc0ba6cadUL, 0xedb88320UL, 0x9abfb3b6UL,
  0x03b6e20cUL, 0x74b1d29aUL, 0xead54739UL, 0x9dd277afUL, 0x04db2615UL,
  0x73dc1683UL, 0xe3630b12UL, 0x94643b84UL, 0x0d6d6a3eUL, 0x7a6a5aa8UL,
  0xe40ecf0bUL, 0x9309ff9dUL, 0x0a00ae27UL, 0x7d079eb1UL, 0xf00f9344UL,
  0x8708a3d2UL, 0x1e01f268UL, 0x6906c2feUL, 0xf762575dUL, 0x806567cbUL,
  0x196c3671UL, 0x6e6b06e7UL, 0xfed41b76UL, 0x89d32be0UL, 0x10da7a5aUL,
  0x67dd4accUL, 0xf9b9df6fUL, 0x8ebeeff9UL, 0x17b7be43UL, 0x60b08ed5UL,
  0xd6d6a3e8UL, 0xa1d1937eUL, 0x38d8c2c4UL, 0x4fdff252UL, 0xd1bb67f1UL,
  0xa6bc5767UL, 0x3fb506ddUL, 0x48b2364bUL, 0xd80d2bdaUL, 0xaf0a1b4cUL,
  0x36034af6UL, 0x41047a60UL, 0xdf60efc3UL, 0xa867df55UL, 0x316e8eefUL,
  0x4669be79UL, 0xcb61b38cUL, 0xbc66831aUL, 0x256fd2a0UL, 0x5268e236UL,
  0xcc0c7795UL, 0xbb0b4703UL, 0x220216b9UL, 0x5505262fUL, 0xc5ba3bbeUL,
  0xb2bd0b28UL, 0x2bb45a92UL, 0x5cb36a04UL, 0xc2d7ffa7UL, 0xb5d0cf31UL,
  0x2cd99e8bUL, 0x5bdeae1dUL, 0x9b64c2b0UL, 0xec63f226UL, 0x756aa39cUL,
  0x026d930aUL, 0x9c0906a9UL, 0xeb0e363fUL, 0x72076785UL, 0x05005713UL,
  0x95bf4a82UL, 0xe2b87a14UL, 0x7bb12baeUL, 0x0cb61b38UL, 0x92d28e9bUL,
  0xe5d5be0dUL, 0x7cdcefb7UL, 0x0bdbdf21UL, 0x86d3d2d4UL, 0xf1d4e242UL,
  0x68ddb3f8UL, 0x1fda836eUL, 0x81be16cdUL, 0xf6b9265bUL, 0x6fb077e1UL,
  0x18b74777UL, 0x88085ae6UL, 0xff0f6a70UL, 0x66063bcaUL, 0x11010b5cUL,
  0x8f659effUL, 0xf862ae69UL, 0x616bffd3UL, 0x166ccf45UL, 0xa00ae278UL,
  0xd70dd2eeUL, 0x4e048354UL, 0x3903b3c2UL, 0xa7672661UL, 0xd06016f7UL,
  0x4969474dUL, 0x3e6e77dbUL, 0xaed16a4aUL, 0xd9d65adcUL, 0x40df0b66UL,
  0x37d83bf0UL, 0xa9bcae53UL, 0xdebb9ec5UL, 0x47b2cf7fUL, 0x30b5ffe9UL,
  0xbdbdf21cUL, 0xcabac28aUL, 0x53b39330UL, 0x24b4a3a6UL, 0xbad03605UL,
  0xcdd70693UL, 0x54de5729UL, 0x23d967bfUL, 0xb3667a2eUL, 0xc4614ab8UL,
  0x5d681b02UL, 0x2a6f2b94UL, 0xb40bbe37UL, 0xc30c8ea1UL, 0x5a05df1bUL,
  0x2d02ef8dUL
};

#define DO1 crc = crc_table[((int)crc ^ (*buf++)) & 0xff] ^ (crc >> 8)
#define DO8 DO1; DO1; DO1; DO1; DO1; DO1; DO1; DO1

uint32_t file::crc32(const unsigned char *buf, size_t len)
{
  uint32_t crc = 0;
  crc = crc ^ 0xffffffffUL;
  while (len >= 8) {
    DO8;
    len -= 8;
  }
  if (len) do {
    DO1;
  } while (--len);
  return crc ^ 0xffffffffUL;
}
