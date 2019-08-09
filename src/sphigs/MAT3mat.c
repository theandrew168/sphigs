/* Copyright 1991, Brown Computer Graphics Group.  All Rights Reserved. */

#ifndef lint
static char Version[] = 
   "$Id: MAT3mat.c,v 1.1 1993/01/14 06:01:02 crb Exp $";
#endif

/* -------------------------------------------------------------------------
 *
 * Contains functions that operate on matrices, but not on matrix-vector pairs,
 * etc. 
 *
 * ------------------------------------------------------------------------- */

#include "mat3defs.h"

/* ------------------------ Static Routines  ------------------------------- */

/* Determinant of 3x3 matrix with given entries */
#define DET3(A,B,C,D,E,F,G,H,I) \
	((A*E*I + B*F*G + C*D*H) - (A*F*H + B*D*I + C*E*G))

/* -------------------------------------------------------------------------
 * DESCR :	Compute a determinant of a 3 x 3 matrix @mat@, multiplied by
 * 		the sign of the permutation (@col1@, @col2@, @col3@). If
 * 		(@col1@, @col2@, @col3@ ) = (0, 1, 2), one gets the ordinary
 * 		determinant.
 *
 * RETURNS :	The determinant.
 * ------------------------------------------------------------------------- */
static double
MAT3_determinant_lower_3(
   MAT3mat mat,	/* Matrix whose determinant we will compute  */
   int	 col1,		/* First entry in permutation */
   int	 col2,		/* 2nd entry in permutation */
   int	 col3 		/* 3rd entry in permutation  */
   )
{
   double det;

   det = DET3(mat[1][col1], mat[1][col2], mat[1][col3],
	      mat[2][col1], mat[2][col2], mat[2][col3],
	      mat[3][col1], mat[3][col2], mat[3][col3]);

   return(det);
}

/* ------------------------ Private Routines ------------------------------- */


/* ------------------------ Public Routines  ------------------------------- */
/*LINTLIBRARY*/

/* -------------------------------------------------------------------------
 * DESCR   :	Sets @m@ to be the identity matrix.
 *
 * RETURNS :	void
 * ------------------------------------------------------------------------- */
void
MAT3identity(
   MAT3mat m 		/* Matrix to be set to identity */
   )
{
    double *mat = (double *)m;

    *mat++ = 1.0;
    *mat++ = 0.0;
    *mat++ = 0.0;
    *mat++ = 0.0;

    *mat++ = 0.0;
    *mat++ = 1.0;
    *mat++ = 0.0;
    *mat++ = 0.0;

    *mat++ = 0.0;
    *mat++ = 0.0;
    *mat++ = 1.0;
    *mat++ = 0.0;

    *mat++ = 0.0;
    *mat++ = 0.0;
    *mat++ = 0.0;
    *mat   = 1.0;
}

/* -------------------------------------------------------------------------
 * DESCR   :	Sets @m@ to be the zero matrix.
 *
 * RETURNS :	void
 * ------------------------------------------------------------------------- */
void
MAT3zero(
   MAT3mat m 		/* Matrix to be set to zero */
   )
{
    double *mat = (double *)m;

    *mat++ = 0.0;
    *mat++ = 0.0;
    *mat++ = 0.0;
    *mat++ = 0.0;

    *mat++ = 0.0;
    *mat++ = 0.0;
    *mat++ = 0.0;
    *mat++ = 0.0;

    *mat++ = 0.0;
    *mat++ = 0.0;
    *mat++ = 0.0;
    *mat++ = 0.0;

    *mat++ = 0.0;
    *mat++ = 0.0;
    *mat++ = 0.0;
    *mat   = 0.0;
}

/* -------------------------------------------------------------------------
 * DESCR   :	Compute the determinant of @mat@.
 *
 * RETURNS :	The determinant, a double.
 * ------------------------------------------------------------------------- */
double
MAT3determinant(
   MAT3mat mat 			/* Matrix whose determinant we compute */
   )
{
   double det;

   det = (  mat[0][0] * MAT3_determinant_lower_3(mat, 1, 2, 3)
	  - mat[0][1] * MAT3_determinant_lower_3(mat, 0, 2, 3)
	  + mat[0][2] * MAT3_determinant_lower_3(mat, 0, 1, 3)
	  - mat[0][3] * MAT3_determinant_lower_3(mat, 0, 1, 2));

   return(det);
}

/* -------------------------------------------------------------------------
 * DESCR   :	Copy the matrix @f@ into the matrix @t@.
 *
 * RETURNS :	void
 * ------------------------------------------------------------------------- */
void
MAT3copy(
   MAT3mat t,	/* Target matrix [output] */
   MAT3mat f	/* Source matrix [input] */
)
{
    double *to = (double *)t;
    double *from = (double *)f;
    
    *to++ = *from++;
    *to++ = *from++;
    *to++ = *from++;
    *to++ = *from++;

    *to++ = *from++;
    *to++ = *from++;
    *to++ = *from++;
    *to++ = *from++;

    *to++ = *from++;
    *to++ = *from++;
    *to++ = *from++;
    *to++ = *from++;

    *to++ = *from++;
    *to++ = *from++;
    *to++ = *from++;
    *to   = *from;
}

/* -------------------------------------------------------------------------
 * DESCR   :	Computes the matrix product @mat1@ * @mat2@ and places the
 *		result in @result_mat@. The matrices being multiplied may be the
 *		same as the result, i.e., a call of the form MAT3mult(a,a,b) is
 *		safe.
 *
 * RETURNS :	void
 * ------------------------------------------------------------------------- */
void
MAT3mult(
   MAT3mat 	 result_mat,	/* The computed product of the next two args  */
   MAT3mat mat1,	/* The first factor of the product */
   MAT3mat mat2 	/* The second factor of the product */
   )
{
   int i, j;
   MAT3mat	tmp_mat;
   double	*tmp;

   if (((tmp = (double*)result_mat) == (double*)mat1) || 
       (tmp == (double*)mat2))
      tmp = (double*)tmp_mat;
   
   for (i = 0; i < 4; i++) for (j = 0; j < 4; j++)
      *(tmp++) = (mat1[i][0] * mat2[0][j] +
		  mat1[i][1] * mat2[1][j] +
		  mat1[i][2] * mat2[2][j] +
		  mat1[i][3] * mat2[3][j]);

   if (((double*)result_mat == (double*)mat1) || 
       ((double*)result_mat == (double*)mat2))
      MAT3copy(result_mat, tmp_mat);

   return;
}

/* -------------------------------------------------------------------------
 * DESCR   :	Compute the transpose of @mat@ and place it in @result_mat@. The
 *		routine may be called with two two arguments equal (i.e.,
 *		MAT3transpose(a, a)) and will still work properly.
 *
 * RETURNS :	void
 * ------------------------------------------------------------------------- */
void
MAT3transpose(
   MAT3mat 	 result_mat,	/* The computed transpose */
   MAT3mat mat 	/* The matrix whose transpose we compute */
   )
{
   int i, j;
   MAT3mat	tmp_mat;

   for (i = 0; i < 4; i++) for (j = 0; j < 4; j++) tmp_mat[i][j] = mat[j][i];

   MAT3copy(result_mat, tmp_mat);
}

/* -------------------------------------------------------------------------
 * DESCR   :	Print the matrix @mat@ on the stream described by the file
 *		point @fp@.
 *
 * RETURNS :	void
 * ------------------------------------------------------------------------- */
void
MAT3print(
   MAT3mat mat,			/* The matrix to be printed */
   FILE	*fp 			/* The stream on which to print it */
   )
{
   MAT3print_formatted(mat, fp, CNULL, CNULL, CNULL, CNULL);
}

/* -------------------------------------------------------------------------
 * DESCR :	This prints the matrix @mat@ to the file pointer @fp@, using
 * 		the format string @format@ to pass to fprintf.  The strings
 * 		@head@ and @tail@ are printed at the beginning and end of
 * 		each line, and the string @title@ is printed at the top.
 *
 * RETURNS :	void
 * ------------------------------------------------------------------------- */
void
MAT3print_formatted(
   MAT3mat mat,			/* Matrix to be printed out */
   FILE	*fp,			/* File stream to which it is printed */
   char	*title,			/* Title of printout */
   char	*head,			/* Printed before each row of matrix */
   char	*format,		/* Format for entries of matrix */
   char	*tail 			/* Printed after each row of matrix */
   )
{
   int i, j;

   /* This is to allow this to be called easily from a debugger */
   if (fp == NULL) fp = stderr;

   if (title  == NULL)	title  = "MAT3 matrix:\n";
   if (head   == NULL)	head   = "  ";
   if (format == NULL)	format = "%#8.4lf  ";
   if (tail   == NULL)	tail   = "\n";

   (void) fprintf(fp, title);

   for (i = 0; i < 4; i++) {
      (void) fprintf(fp, head);
      for (j = 0; j < 4; j++) (void) fprintf(fp, format, mat[i][j]);
      (void) fprintf(fp, tail);
   }
}

/* -------------------------------------------------------------------------
 * DESCR :	This compares two matrices @m1@ and @m2@ for equality
 * 		within @epsilon@.  If all corresponding elements in the
 * 		two matrices are equal within @epsilon@, then TRUE is
 * 		returned.  Otherwise, FALSE is returned. Epsilon should be
 * 		zero or positive.
 *
 * RETURNS :	TRUE if all entries differ pairwise by less than epsilon,
 * 		FALSE otherwise.
 * ------------------------------------------------------------------------- */
BOOL
MAT3equal(
   MAT3mat m1,		/* The first matrix to be compared */
   MAT3mat m2,		/* The second matrix to be compared */
   double	epsilon 	/* The value within which entries must match */
   )
{
   int i, j;
   double	diff;

   /* Special-case for epsilon of 0 */
   if (epsilon == 0.0) {
      for (i = 0; i < 4; i++) for (j = 0; j < 4; j++)
	 if (m1[i][j] != m2[i][j]) return(FALSE);
   }

   /* Any other epsilon */
   else for (i = 0; i < 4; i++) for (j = 0; j < 4; j++) {
      diff = m1[i][j] - m2[i][j];
      if (! IS_ZERO(diff, epsilon)) return(FALSE);
   }

   return TRUE;
}

/* -------------------------------------------------------------------------
 * DESCR   :	Compute the trace of @mat@ (i.e., sum of diagonal entries).
 *
 * RETURNS :	The trace of @mat@. 
 * ------------------------------------------------------------------------- */
double
MAT3trace(
   MAT3mat mat		/* Matrix whose trace we are computing */
)
{
   return(mat[0][0] + mat[1][1] + mat[2][2] + mat[3][3]);
}

/* -------------------------------------------------------------------------
 * DESCR   :	Raises @mat@ to the power @power@, placing the result in
 * 		@result_mat@.
 *
 *
 *
 * RETURNS :	TRUE if the power is non-negative, FALSE otherwise
 *
 * DETAILS :	The computation is performed by starting with the identity
 * 		matrix as the result. Then look at the bits of the binary
 * 		representation of the power, from left to right. Every time a
 * 		1 is encountered, we square the matrix and then multiply the
 * 		result by the original matrix. Every time that a 0 is
 * 		encountered, just square the matrix.  When done with all the
 * 		bits in the power, the matrix will be the correct result
 * ------------------------------------------------------------------------- */
int
MAT3power(
   MAT3mat result_mat,		/* The source matrix raised to the power */
   MAT3mat mat,			/* The source matrix */
   int	power 			/* The power to which to raise the source */
   )
{
   int count, rev_power;
   MAT3mat	temp_mat;

   if (power < 0) {
      SEVERES("Negative power given to MAT3power:");
      return(FALSE);
   }
   /* Reverse the order of the bits in power to make the looping construct
    * effiecient (and word-size-independent). */
   for (rev_power = count = 0; power > 0; count++, power = power >> 1)
	 rev_power = (rev_power << 1) | (power & 1);
   power = rev_power;

   MAT3identity(temp_mat);

   /* For each bit in power */
   for ( ; count > 0 || rev_power != 0; count--, rev_power = rev_power >> 1) {

      /* Square working matrix */
      MAT3mult(temp_mat, temp_mat, temp_mat);

      /* If bit is 1, multiply by original matrix */
      if (power & 1) MAT3mult(temp_mat, temp_mat, mat);
   }

   MAT3copy(result_mat, temp_mat);

   return (TRUE);
}


/* -------------------------------------------------------------------------
 * DESCR :	Computes the column-reduced form of @mat@ in @result_mat@.
 * 		After column reduction, each column's first non-zero entry is
 * 		a one.  The leading ones are ordered highest to lowest from
 * 		the first column to the last. 
 *
 * RETURNS :	The rank of the matrix, i.e., the number of linearly
 * 		independent columns of the original matrix.
 *
 * DETAILS :	Performs the reduction through simple column operations:
 * 		multiply a column by a scalar to make its leading value one,
 * 		then add a multiple of this column to each of the others to
 * 		put zeroes in all other entries of that row. The number
 * 		@epsilon@ is used to test near equality with zero.  It should
 * 		be non-negative.
 * ------------------------------------------------------------------------- */
int
MAT3column_reduce(
   MAT3mat result_mat,	/* The column-reduced matrix [output] */
   MAT3mat 	 mat,		/* The original matrix [input] */
   double	 epsilon 	/* The tolerance for testing against 0 */
   )
{
   int row, col, next_col, r;
   double	t, factor;

   MAT3copy(result_mat, mat);

   /* next_col is the next column in which a leading one will be placed */
   next_col = 0;
   for (row = 0; row < 4; row++) {
      for (col = next_col; col < 4; col++)
	 if (! IS_ZERO(result_mat[row][col], epsilon)) break;

      if (col == 4) continue;			/* all 0s, try next row */
      if (col != next_col)			/* swap cols */
	 for (r = row; r < 4; r++)
	    SWAP(result_mat[r][col], result_mat[r][next_col], t);

      /* Make a leading 1 */
      factor = 1.0 / result_mat[row][next_col]; /* make a leading 1 */
      result_mat[row][next_col] = 1.0;
      for (r = row + 1; r < 4; r++) result_mat[r][next_col] *= factor;

      for (col = 0; col < 4; col++)		/* zero out cols */
	 if (col != next_col) { 		/* could skip if already zero */
	 factor = -result_mat[row][col];
	 result_mat[row][col] = 0.0;
	 for (r = row + 1; r < 4; r++)
	    result_mat[r][col] += factor * result_mat[r][next_col];
      }

      if (++next_col == 4) break;
   }

   return(next_col);
}


/* -------------------------------------------------------------------------
 * DESCR :	This computes the kernel of the transformation represented by
 * 		@mat@ from its column-reduced form, and places a basis for
 * 		the kernel in the rows of @result_mat@. The number "epsilon"
 * 		is used to test near-equality with zero. It should be
 * 		non-negative.  If there is no basis (i.e., the matrix is
 * 		non-singular), this returns FALSE.
 *
 *
 * RETURNS :	TRUE if the nullity is nonzero, FALSE otherwise. 
 * ------------------------------------------------------------------------- */
int
MAT3kernel_basis(
   MAT3mat result_mat,	/* Stores basis of kernel of mat [output] */
   MAT3mat 	 mat,		/* Matrix whose kernel we compute [input] */
   double	 epsilon 	/* Tolerance for comparison with zero [input] */
   )
{
   MAT3mat	reduced;
   int		leading[4],	/* Indices of rows with leading ones	*/
		other[4],	/* Indices of other rows		*/
		rank,		/* Rank of column-reduced matrix	*/
		dim;		/* Dimension of kernel (4 - rank)	*/
   int lead_count, other_count, r, c;

   /* Column-reduce the matrix */
   rank = MAT3column_reduce(reduced, mat, epsilon);

   /* Get dimension of kernel. If zero, there is no basis */
   if ((dim = 4 - rank) == 0) return(FALSE);

   /* If matrix transforms all points to origin, return identity */
   if (rank == 0) MAT3identity(result_mat);

   else {
      /* Find rows with leading ones, and other rows */
      lead_count = other_count = 0;
      for (r = 0; r < 4; r++) {
	 if (reduced[r][lead_count] == 1.0) leading[lead_count++] = r;
	 else				    other[other_count++]  = r;
      }

      /* Put identity into rows with no leading ones */
      for (r = 0; r < dim; r++) for (c = 0; c < other_count; c++)
	 result_mat[r][other[c]] = (r == c ? -1.0 : 0.0);

      /* Copy other values */
      for (r = 0; r < dim; r++) for (c = 0; c < rank; c++)
	 result_mat[r][leading[c]] = reduced[other[r]][c];
   }
   return(TRUE);
}

/* -------------------------------------------------------------------------
 * DESCR   :	Multiply matrix @m@ by the real number @scalar@ and put the
 * 		result in @result_mat@.  
 *
 * RETURNS :	void
 * ------------------------------------------------------------------------- */
void
MAT3scalar_mult(
   MAT3mat result_mat,	/* The result of scaling the matrix [output] */
   MAT3mat          m,		/* Matrix to be scaled [input] */
   double           scalar 	/* Scaling factor [input] */
   )
{
   int i,j;

   for (i = 0; i < 4; i++) for (j = 0; j < 4; j++) {
      result_mat[i][j] = scalar * m[i][j];
   }
}

/* -------------------------------------------------------------------------
 * DESCR   :	Compute the sum of matrices @A@ and @B@ and place the result
 * 		in @result_mat@, which may be one of @A@ and @B@ without any
 * 		fear of errors.
 *
 * RETURNS :	void
 * ------------------------------------------------------------------------- */
void
MAT3mat_add(
   MAT3mat result_mat,	/* The sum of the matrices [output] */
   MAT3mat A,			/* The first summand [input] */
   MAT3mat B 			/* The second summand [input] */
   )
{
   int i,j;

   for (i = 0; i < 4; i++) for (j = 0; j < 4; j++) {
      result_mat[i][j] = A[i][j] + B[i][j];
   }
}
