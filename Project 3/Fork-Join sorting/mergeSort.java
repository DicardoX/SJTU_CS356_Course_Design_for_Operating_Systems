/**
*	Fork - Join Merge Sorting Task.
*	Created by Chunyu Xue on May, 2020
**/

import java.util.Scanner;
import java.util.Arrays;
import java.util.concurrent.*;
import java.io.*;

public class MergeSort extends RecursiveAction
{
	static int Size = 1000;
	static int Threshold = 10;
	public static final int MAXSIZE = 500;

	private int left;
	private int right;
	private int[] arr;

	public MergeSort(int left, int right, int[] arr){
		this.left = left;
		this.right = right;
		this.arr = arr;
	} 

	/** Override compute() */
	protected void compute()					
	{
		if(right - left + 1 < Threshold)				// Use bubble sort
		{
			boolean swapped;
			for(int i=left; i < right; i++){
				swapped = false;
				for(int j = left; j < right - i; j++){
					if(arr[j] > arr[j+1]){
						int tmp = arr[j];				// Swap
						arr[j] = arr[j+1];
						arr[j+1] = tmp;
						swapped = true;
					}
				}
				if(!swapped) break;
			}
		}
		else 											// Use quick merge sort
		{
			int mid = (left + right) / 2;
			MergeSort lTask = new MergeSort(left, mid, arr);
			MergeSort rTask = new MergeSort(mid+1, right, arr);

			/** Fork */
			lTask.fork();
			rTask.fork();
			/** Join */
			lTask.join();
			rTask.join();
			/** Merge */
			merge(arr, left, right);
		}
	}

	protected void merge(int[] arr, int l, int r)
	{
		int[] tmp_arr = new int[r - l + 1];
		int mid = (l + r) / 2;
		int left = l;
		int right = r;
		int vis = 0;

		while(left <= mid && right <= r)
		{
			// Compare 
			if(arr[left] < arr[right]){
				tmp_arr[vis++] = arr[left++];
			}
			else{
				tmp_arr[vis++] = arr[right++];
			}
			// Rest part
			while(left <= mid){
				tmp_arr[vis++] = arr[left++];
			}
			while(right <= r){
				tmp_arr[vis++] = arr[right++];
			}
			// Modify intial array
			for(int i = 0; i < r - l + 1; i++){
				arr[l + i] = tmp_arr[i];
			}
		}
	}

	public static void main(String [] args) throws IOException
	{
		ForkJoinPool pool = new ForkJoinPool();
		char cmd;

		/** Scanner definition */
		Scanner input = new Scanner(System.in);			

		/** Initial set */
		System.out.println("Please choose whether randomly generete the array (input Y) or self input the array (input N):");
		cmd = input.next().charAt(0);

		System.out.println("Please input the size of the randomly genereted array:");
		Size = input.nextInt();

		int [] arr = new int[Size];

		switch(cmd){
			case 'Y':

				int times = 100000;									// Loop times
				while((times--) != 0)
				{
					java.util.Random rand = new java.util.Random();
					for(int i=0; i < Size; i++){
						arr[i] = rand.nextInt(MAXSIZE*2) - MAXSIZE;		// Randomly generated
					}
					System.out.println("The initial array is:");
					System.out.println(Arrays.toString(arr));

					MergeSort task = new MergeSort(0, Size - 1, arr);

					/** Invoke task */		
					pool.invoke(task);											

					/** Print result */
					System.out.println("The sorted array is:");
					System.out.println(Arrays.toString(arr));

					/** Check correctness */
					for(int i = 1; i < Size; i++){
						if(arr[i-1] > arr[i]){
							System.out.println("Error!");
							System.exit(0);
						}
					}
					System.out.println("--------Round "+ (100000-times) + ": Success!--------");
				}
				break;

			case 'N':
				System.out.println("Please input the array:");
				BufferedReader br = new BufferedReader(new InputStreamReader(System.in));
				String str;
				str = br.readLine();
				String [] str_arr = str.split(" ");
				for(int i=0; i < Size; i++){
					arr[i] = Integer.parseInt(str_arr[i]);			// Transfer into integer array
				}

				/** Function */
				MergeSort task = new MergeSort(0, Size - 1, arr);

				/** Invoke task */		
				pool.invoke(task);											

				/** Print result */
				System.out.println("The sorted array is:");
				System.out.println(Arrays.toString(arr));
				break;

			default:
				break;
		}

	}
}
