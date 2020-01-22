#include <stdio.h> 
#include <stdlib.h> 

int codelen(char *);
void strconcat(char *,char *, char);
 
int main() 
{ 
	int i, j;  
	int data = 0;
	int offset, width, height; ;
	long bmpsize = 0, bmpdataoff = 0; 
	int** image; 
	int temp = 0; 

	// reading the .bmp file 
	FILE* image_file; 
	
	image_file = fopen("7.bmp", "rb"); 
	if (image_file == NULL) 
	{ 
		printf("Error Opening File!!"); 
		exit(1); 
	} 
	else
	{ 
		//according to bmp header format 
		offset = 0; 
		
		//1st and 2nd bytes are "BM" 
		offset = 2; 
		
		fseek(image_file, offset, SEEK_SET); 
		
		// getting size of .bmp file in 4 bytes
		fread(&bmpsize, 4, 1, image_file); 
		
		// getting offset where the image data starts
		offset = 10; 
		
		fseek(image_file, offset, SEEK_SET);  
		fread(&bmpdataoff, 4, 1, image_file); 
		
		// width is stored at offset 18 and height at offset 22 (each of 4 bytes)
		fseek(image_file, 18, SEEK_SET); 
		fread(&width, 4, 1, image_file); 
		fread(&height, 4, 1, image_file); 
		
		// number of bits per pixel at offset 28
		fseek(image_file, 2, SEEK_CUR); 
		fread(&bpp, 2, 1, image_file); 
		printf("%d\n",bpp);
		
		// setting offset to start of image data 
		fseek(image_file, bmpdataoff, SEEK_SET); 
		
		// creating image array 
		image = (int**)malloc(height * sizeof(int*)); 
		
		for (i = 0; i < height; i++) 
		{ 
			image[i] = (int*)malloc(width * sizeof(int)); 
		} 
		
		//read the data into image array
		for (i = 0; i < height; i++) 
		{ 
			for (j = 0; j < width; j++) 
			{ 
				//number of bytes = 24/8 = 3 as the image is a 24-bits per pixel image
				fread(&temp, 3, 1, image_file); 
				temp = temp & 0x0000FF; 
				image[i][j] = temp; 
			} 
		} 
	}
	
	// finding the frequency of each pixel value (0-255) 
	int prob[256]; 
	for (i = 0; i < 256; i++) 
		prob[i] = 0; 
	for (i = 0; i < height; i++) 
		for (j = 0; j < width; j++) 
			prob[image[i][j]] += 1; 
			
	//finding the number of non-zero occurences which is equal to the number of leaf nodes
	int nodes = 0; 
	for (i = 0; i < 256; i++) 
		if (prob[i] != 0) 
			nodes += 1; 
	
	//defining structures  
	struct freq_tree 
	{ 
		int pix;
		float freq; 
		struct freq_tree *left, *right; 
		char code[2000]; 
	}; 

	struct huff_code 
	{ 
		int pix, arrloc; 
		float freq; 
	}; 
	
	struct freq_tree* freq_trees; 
	struct huff_code* huff_codes; 
	//total number of nodes = 2*leaf nodes - 1
	int totalnodes = 2 * nodes - 1; 
	freq_trees = (struct freq_tree*)malloc(sizeof(struct freq_tree) * totalnodes); 
	huff_codes = (struct huff_code*)malloc(sizeof(struct huff_code) * nodes); 
	

	j = 0; 
	int tot_size = height * width; 
	float tempprob; 
	for (i = 0; i < 256; i++) 
	{ 
		if (prob[i] != 0) 
		{ 
			
			// pixel intensity value 
			huff_codes[j].pix = i; 
			freq_trees[j].pix = i; 
			
			// location of the node in the freq_trees array 
			huff_codes[j].arrloc = j;
			
			// probability of occurrence 
			tempprob = (float)prob[i] / (float)tot_size; 
			freq_trees[j].freq = tempprob; 
			huff_codes[j].freq = tempprob; 
			
			freq_trees[j].left = NULL; 
			freq_trees[j].right = NULL; 

			freq_trees[j].code[0] = '\0'; 
			j++; 
		} 
	} 
 
	struct huff_code temphuff; 
	
	// sort the huff_codes in descending order w.r.t probability  of occurence (insertion sort)
	for (i = 0; i < nodes; i++) 
	{ 
		for (j = i + 1; j < nodes; j++) 
		{ 
			if (huff_codes[i].freq < huff_codes[j].freq) 
			{ 
				temphuff = huff_codes[i]; 
				huff_codes[i] = huff_codes[j]; 
				huff_codes[j] = temphuff; 
			} 
		} 
	} 
	
	// building huffman tree
	float sumprob; 
	int sumpix; 
	int n = 0, k = 0; 
	int nextnode = nodes; 

 
	while (n < nodes - 1) 
	{ 
		
		// adding the lowest two probabilities 
		sumprob = huff_codes[nodes - n - 1].freq + huff_codes[nodes - n - 2].freq; 
		sumpix = huff_codes[nodes - n - 1].pix + huff_codes[nodes - n - 2].pix; 
		
		// Appending to the freq_trees Array 
		freq_trees[nextnode].pix = sumpix; 
		freq_trees[nextnode].freq = sumprob; 
		freq_trees[nextnode].left = &freq_trees[huff_codes[nodes - n - 2].arrloc]; 
		freq_trees[nextnode].right = &freq_trees[huff_codes[nodes - n - 1].arrloc]; 
		freq_trees[nextnode].code[0] = '\0'; 
		i = 0; 
		
		while (sumprob <= huff_codes[i].freq) 
			i++; 
			
		// inserting the new node in the huff_codes array 
		for (k = nodes; k >= 0; k--) 
		{ 
			if (k == i) 
			{ 
				huff_codes[k].pix = sumpix; 
				huff_codes[k].freq = sumprob; 
				huff_codes[k].arrloc = nextnode; 
			} 
			else if (k > i) 
				huff_codes[k] = huff_codes[k - 1]; 
			
		} 
		n += 1; 
		nextnode += 1; 
	} 
	
	//assigning code using backtrackings 
	char left = '0'; 
	char right = '1'; 
	int index; 
	for (i = totalnodes - 1; i >= nodes; i--) 
	{ 
		if (freq_trees[i].left != NULL) 
			strconcat(freq_trees[i].left->code, freq_trees[i].code, left); 
		if (freq_trees[i].right != NULL) 
			strconcat(freq_trees[i].right->code, freq_trees[i].code, right); 
	} 
	
	int pix_val; 
	int l; 
	
	// writing the huffman encoded image into a text file 
	FILE* imagehuff = fopen("encoded_image1.txt", "wb"); 
	for (i = 0; i < height; i++) 
		for (j = 0; j < width; j++) 
		{ 
			pix_val = image[i][j]; 
			for (l = 0; l < nodes; l++) 
				if (pix_val == freq_trees[l].pix) 
					fprintf(imagehuff, "%s", freq_trees[l].code);
		} 
		
 
	printf("Huffmann Codes:\n\n"); 
	printf("pixel values -> Code\n\n"); 
	for (i = 0; i < nodes; i++) { 
		printf(" %d	 -> %s\n", freq_trees[i].pix, freq_trees[i].code); 
	} 
	
	//average bit length 
	float avgbitnum = 0; 
	for (i = 0; i < nodes; i++) 
		avgbitnum += freq_trees[i].freq * codelen(freq_trees[i].code); 
	printf("Average number of bits: %f", avgbitnum); 
} 
//find the length of code
int codelen(char* code) 
{ 
	int l = 0; 
	while (*code != '\0') 
		code = code + l;
		l++; 
	return l; 
} 

//concatenate code of different nodes
void strconcat(char* str, char* parentcode, char add) 
{ 
	int i = 0; 
	while (*(parentcode + i) != '\0') 
	{ 
		*(str + i) = *(parentcode + i); 
		i++; 
	} 
	if (add != '2') 
	{ 
		str[i] = add; 
		str[i + 1] = '\0'; 
	} 
	else
		str[i] = '\0'; 
}