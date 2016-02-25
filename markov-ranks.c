#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<time.h>

/*	input format:
 *	winning team points losing team points
 *
 *	ex:
 *	Warriors 110 Suns 55
 *	Clippers 95 Lakers 22
 */

#define GAMES_PER_SEASON 82
#define NUM_STEPS 100000
#define WORDS_IN_TEAM_NAME 0 //this is the sixers clause. 0 for college 1 for NBA

char **findTeamsList(FILE *inData, int *num);
void parseIn(char *line, char *team1, char *team2, int *score1, int *score2);
int isContained(char **teamList, char *teamName);
void formMatrix(FILE *inData, char **teamList);
void incrementMatrix(char **list, char *winning, char *losing);
void randomWalk(int *visits, int num);
int randomlySelect(int *row, int num);


//from first index to second index
//weights are number of times first index has has lost to second index x number of times
int **adjMatrix;

int main(int argc, char *argv[]){
	if(argc != 2){
		printf("Usage: ./markov-ranks filename\n");
		exit(0);
	}

	int num = 0;
	int i, j;
	FILE *inData = fopen(argv[1], "r");
	srand48(clock());

	//find list of teams
	char **teams = findTeamsList(inData, &num);
	
	//create the adjacency matrix
	adjMatrix = (int **) malloc(sizeof(int *)*num);
	for(i=0; i<num; i++){
		adjMatrix[i] = (int *) malloc(sizeof(int)*num);
		for(j=0; j<num; j++) adjMatrix[i][j] = 0;
	}

	fclose(inData);
	inData = fopen(argv[1], "r");

	//form adj matrix
	formMatrix(inData, teams);

	//perform the random walk
	int *visits = (int *) calloc(num, sizeof(int));
	randomWalk(visits, num);

	for(i=0; i<num; i++){
		printf("%s\t\t", teams[i]);
		if(strlen(teams[i])<16) printf("\t");
		printf("%d\n", visits[i]);
	}
	return 0;
}

void randomWalk(int *visits, int num){
	int i;
	int currentTeam = drand48()*num;


	for(i=0; i<NUM_STEPS; i++){
		visits[currentTeam]++;
		currentTeam = randomlySelect(adjMatrix[currentTeam], num);
	}
}

int randomlySelect(int *row, int num){
	int *losses = (int *) malloc(sizeof(int)*(GAMES_PER_SEASON+1));
	int i, j;
	int n=0; //index into the losses list

	//moves you through the row
	for(i=0; i<num; i++){
		//adds one to the losses list for each in this spot
		for(j=0; j<row[i]; j++){
			losses[n] = i;
			n++;
		}
	}

	if(n==0){
		return (int) (drand48()*num);
	} else {
		return losses[(int) (drand48()*n)];
	}
}

void formMatrix(FILE *inData, char **teamList){
	char *line = malloc(sizeof(char)*80);
	char *team1 = (char *) malloc(sizeof(char)*30);
	char *team2 = (char *) malloc(sizeof(char)*30);
	int score1, score2;

	while(fgets(line, 80, inData) != NULL){
		parseIn(line, team1, team2, &score1, &score2);
		
		if(score1 > score2){
			//team1 beat team2
			incrementMatrix(teamList, team1, team2);
		} else {
			//team2 beat team1
			incrementMatrix(teamList, team2, team1);
		}
	}
}

void incrementMatrix(char **list, char *winning, char *losing){
	int winN, loseN;
	int i = 0;

	while(1) if(strcmp(list[i++], winning) == 0) break;
	winN = i-1;

	i=0;
	while(1) if(strcmp(list[i++], losing) == 0) break;
	loseN = i-1;

	adjMatrix[loseN][winN]++;
}

char **findTeamsList(FILE *inData, int *num){
	int n = *num;
	char *line = malloc(sizeof(char)*80);
	char *team1 = (char *) malloc(sizeof(char)*30);
	char *team2 = (char *) malloc(sizeof(char)*30);
	char **teamList = (char **) calloc(10, sizeof(char *));
	int score1, score2;

	while(fgets(line, 80, inData) != NULL){
		parseIn(line, team1, team2, &score1, &score2);
		if(isContained(teamList, team1) == 0){
			teamList[n++] = team1;
			team1 =  (char *) malloc(sizeof(char)*30);
		}

		if(n%10 == 9){
			teamList = realloc(teamList, (n+11)*sizeof(char *));
			int z;
			for(z=n; z<n+10; z++) teamList[z] = NULL;
		}

		if(isContained(teamList, team2) == 0){
			teamList[n++] = team2;
			team2 =  (char *) malloc(sizeof(char)*30);
		}

		if(n%10 == 9){
			teamList = realloc(teamList, (n+11)*sizeof(char *));
			int z;
			for(z=n; z<n+10; z++) teamList[z] = NULL;
		}
	}

	*num = n;
	return teamList;
}


void parseIn(char *line, char *team1, char *team2, int *score1, int *score2){
	int i=0;
	int n=0;
	char cscore1[5];
	char cscore2[5];

	//first team name
	while(1){
		if(line[i] < 0x41 && line[i] != 0x20 && n>WORDS_IN_TEAM_NAME) break;
		if(line[i] == 0x20) n++;
		team1[i] = line[i];
		i++;
	}
	team1[i-1] = 0x00;

	//get the first score
	int lengthSoFar = i;
	while(line[i] <= 0x39){
		cscore1[i-lengthSoFar] = line[i];
		i++;
	}
	cscore1[i-lengthSoFar] = 0x00;
	*score1 = atoi(cscore1);

	//second team name
	lengthSoFar = i;
	n=0;
	while(1){
		if((line[i] < 0x41 && line[i] != 0x20) && n>WORDS_IN_TEAM_NAME) break;
		if(line[i] == 0x20) n++;
		team2[i-lengthSoFar] = line[i];
		i++;
	}
	team2[i-1-lengthSoFar] = 0x00;

	//get the first score
	lengthSoFar = i;
	while(line[i] <= 0x39 && line[i] != 0x00){
		cscore2[i-lengthSoFar] = line[i];
		i++;
	}
	cscore2[i-lengthSoFar] = 0x00;
	*score2 = atoi(cscore2);
}

//returns:
// 1 if teamName is contained in teamList
// 0 if it is not
int isContained(char **teamList, char *teamName){
	int i = 0;
	while(teamList[i]!=NULL){
		if(strcmp(teamList[i++], teamName) == 0) return 1;
	}
	return 0;
}
