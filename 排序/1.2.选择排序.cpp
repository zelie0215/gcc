#include <stdio.h>
int main()
{
    int i,j,t,a[] = { 22, 34, 3, 32, 82, 55, 89, 50, 37, 5, 64, 35, 9, 70 };;    //定义变量及数组为基本整型
    for(i=1;i<=9;i++)
        for (j=i+1;j<=10;j++)
            if(a[i]>a[j])    //如果前一个数比后一个数大，则利用中间变量t实现两值互换
            {
                t=a[i];
                a[i]=a[j];
                a[j]=t;
            }
    printf("排序后的顺序是：\n");
    for(i=1;i<=10;i++)
        printf("%5d", a[i]);    //输出排序后的数组
    printf("\n");
    return 0;
}