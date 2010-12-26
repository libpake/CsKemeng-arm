#include "ini.h"
#include "basetype.h"

 void list_append(INI *pini,
		char *key,
		char *value,
		char *section
		)
{
	pthread_mutex_lock(&malloc_flag_mutex);
	ini_item *item = (ini_item *)malloc(sizeof(ini_item));
	pthread_mutex_unlock(&malloc_flag_mutex);

	item->section = section;
	item->key = key;
	item->value = value;
	item->next = NULL;
	if (!pini->tailer) {
		pini->header = pini->tailer = item;
	} else {
		pini->tailer->next = item;
		pini->tailer = item;
	}
	pini->length += 1;

	return;
}

ini_item *list_search(INI *pini,const char *key)
{
	ini_item *p = pini->header;
	char temp[VCHR_LEN];
	char *k = temp, *sect=temp;
	ini_item *_ret = NULL;

	if (!p) return NULL;
	strcpy(temp,key);
	for (; *k && '.'!=*k; k++);
	if ('.'==*k && k==sect)
		sect = ++k;
	else if ('.' == *k)
		*k++ = 0;
	else
		k = sect;

	if (k!=sect && (unsigned int)(k-sect)==strlen(key)) {
		while (p) {
			if (0 != strcmp(p->section,sect)) {
				p = p->next;
				continue;
			}
			if (!p->next)
				return p;
			if (0 != strcmp(p->next->section,sect))
				return p;
			p = p->next;
		}
		return _ret;
	}

	while (p) {
		if (0 != strcmp(p->key,k)) {
			p = p->next;
			continue;
		}
		if (k!=sect && 0!=strcmp(p->section,sect)) {
			p = p->next;
			continue;
		}
		if (k==sect && 0!=strcmp(p->section,
				pini->header->section)) {
			p = p->next;
			continue;
		}
		_ret = p;
		break;
	}

	return _ret;
}

INI *ini_open(const char *fname)
{
	INI *pini;
	FILE *fp = fopen(fname,"r");
	//char buffer[LINE_LEN];
	char *buffer;
	char *p;
	char *s;
	char sect[SECT_LEN] = DEF_SECT_ROOT;
	char *section;
	char *key;
	char *value;

	buffer = (char *)malloc(10240);
	if (!buffer) return NULL;
	bzero(buffer, 10240);
	if (!fp) return NULL;
	pthread_mutex_lock(&malloc_flag_mutex);
	pini = (INI *)malloc(sizeof(INI));
	pthread_mutex_unlock(&malloc_flag_mutex);

	pthread_mutex_lock(&malloc_flag_mutex);
	pini->fname = (char *)malloc(strlen(fname)+1);
	pthread_mutex_unlock(&malloc_flag_mutex);
	strcpy(pini->fname,fname);
	pini->length = 0;
	pini->header = pini->tailer = NULL;

	while (!feof(fp)) {
		if (!fgets(buffer,20480,fp))
			break;
		for (p=buffer; ' '==*p||'\t'==*p; p++);
		if ('#'==*p || '\n'==*p || '\r'==*p) {
			continue;
		} else if ('[' == *p) {
			for (p++; ' '==*p||'\t'==*p; p++);
			for (s=p; ' '!=*p&&'\t'!=*p&&']'!=*p; p++);
			*p = 0;
			strcpy(sect,s);
		} else {
			for (; ' '==*p||'\t'==*p; p++);
			for (s=p; ' '!=*p&&'\t'!=*p&&'='!=*p&&':'!=*p; p++);
			*p = 0;
			pthread_mutex_lock(&malloc_flag_mutex);
			key = (char *)malloc(strlen(s)+1);
			pthread_mutex_unlock(&malloc_flag_mutex);
			strcpy(key,s);

			for (p++; ' '==*p||'\t'==*p||'='==*p||':'==*p; p++);
			//for (s=p; ' '!=*p&&'\t'!=*p&&'\n'!=*p&&'\r'!=*p; p++);
			for (s=p; '\n'!=*p&&'\r'!=*p; p++);
			*p = 0;
			pthread_mutex_lock(&malloc_flag_mutex);
			value = (char *)malloc(strlen(s)+1);
			pthread_mutex_unlock(&malloc_flag_mutex);
			strcpy(value,s);

			pthread_mutex_lock(&malloc_flag_mutex);
			section = (char *)malloc(strlen(sect)+1);
			pthread_mutex_unlock(&malloc_flag_mutex);
			strcpy(section,sect);

			list_append(pini,key,value,section);
		}
	}
	fclose(fp);
	pthread_mutex_lock(&malloc_flag_mutex);
	free(buffer);
	pthread_mutex_unlock(&malloc_flag_mutex);
	return pini;
}

INI *ini_create(const char *fname)
{
	FILE *fp = fopen(fname,"w+");
	INI *pini;

	if (!fp) return NULL;

	pthread_mutex_lock(&malloc_flag_mutex);
	pini = (INI *)malloc(sizeof(INI));
	pthread_mutex_unlock(&malloc_flag_mutex);

	pthread_mutex_lock(&malloc_flag_mutex);
	pini->fname = (char *)malloc(strlen(fname));
	pthread_mutex_unlock(&malloc_flag_mutex);
	strcpy(pini->fname,fname);
	pini->length = 0;
	pini->header = pini->tailer = NULL;

	fclose(fp);

	return pini;
}

#if 0
ini ini_save_ex(INI *pini, char *key)
{
	FILE *fp;
	ini_item *item;
	char *sect = NULL, *buffer, *p, *s;
	int  max_len;

	item = pini->header;
	if (!item) return 0;
	fp = fopen(pini->fname,"w+");
	if (!fp) return 0;

	while (!feof(fp)) {
		if (!fgets(buffer,20480,fp))
			break;
		for (p=buffer; ' '==*p||'\t'==*p; p++);
		if ('#'==*p || '\n'==*p || '\r'==*p) {
			continue;
		} else if ('[' == *p) {
			for (p++; ' '==*p||'\t'==*p; p++);
			for (s=p; ' '!=*p&&'\t'!=*p&&']'!=*p; p++);
			*p = 0;
			strcpy(sect,s);
		} else {
			for (; ' '==*p||'\t'==*p; p++);
			for (s=p; ' '!=*p&&'\t'!=*p&&'='!=*p&&':'!=*p; p++);
			*p = 0;
			pthread_mutex_lock(&malloc_flag_mutex);
			key = (char *)malloc(strlen(s)+1);
			pthread_mutex_unlock(&malloc_flag_mutex);
			strcpy(key,s);

			for (p++; ' '==*p||'\t'==*p||'='==*p||':'==*p; p++);
			//for (s=p; ' '!=*p&&'\t'!=*p&&'\n'!=*p&&'\r'!=*p; p++);
			for (s=p; '\n'!=*p&&'\r'!=*p; p++);
			*p = 0;
			pthread_mutex_lock(&malloc_flag_mutex);
			value = (char *)malloc(strlen(s)+1);
			pthread_mutex_unlock(&malloc_flag_mutex);
			strcpy(value,s);

			pthread_mutex_lock(&malloc_flag_mutex);
			section = (char *)malloc(strlen(sect)+1);
			pthread_mutex_unlock(&malloc_flag_mutex);
			strcpy(section,sect);


			}
		}

}
#endif
int ini_save(INI *pini)
{
	FILE *fp;
	ini_item *p;
	char *sect = NULL, *buffer;
	int  max_len;



	p = pini->header;
	if (!p) return 0;
	fp = fopen(pini->fname,"w+");
	if (!fp) return 0;

	while (p) {
		if (!sect || 0!=
		strcmp(p->section,sect)) {
            max_len = strlen(p->key)  +strlen("\t= ") + strlen(p->value) + strlen("\n") +10 ;
			pthread_mutex_lock(&malloc_flag_mutex);
            buffer = (char *)malloc(max_len * sizeof(char));
			pthread_mutex_unlock(&malloc_flag_mutex);
            if (!buffer)  return 0;
			sect = p->section;
			buffer[0] = '[';
			strcpy(buffer+1,sect);
			strcat(buffer,"]\n");
			fputs(buffer,fp);
		}
        if(!buffer){
            max_len = strlen(p->key)  +strlen("\t= ") + strlen(p->value) + strlen("\n") +10;
			pthread_mutex_lock(&malloc_flag_mutex);
            buffer = (char *)malloc(max_len * sizeof(char));
			pthread_mutex_unlock(&malloc_flag_mutex);
            if (!buffer)  return 0;
        }
		strcpy(buffer,p->key);
		strcat(buffer,"\t= ");
		strcat(buffer,p->value);
		strcat(buffer,"\n");
		fputs(buffer,fp);

		p = p->next;
        free(buffer);
        buffer = NULL;
	}

	fclose(fp);

	return 1;
}

void ini_close(INI *pini)
{
	ini_item *p = pini->header;
	ini_item *temp;

	if (!p) return;
	while (p) {
		free(p->key);
		free(p->value);
		free(p->section);
		temp = p;
		p = p->next;
		free(temp);
	}

	free(pini->fname);
	free(pini);

	return;
}

char *ini_get(INI *pini,const char *key,char *value)
{
	ini_item *item;

	item = list_search(pini,key);
	*value = 0;
	if (item) strcpy(value,item->value);

	return value;
}

int ini_get_int(INI *pini,const char *key)
{
	char value[VINT_LEN];
	return atoi(ini_get(pini,key,value));
}

int ini_set(INI *pini,const char *key,const char *value)
{
	ini_item *item;

	item = list_search(pini,key);
	if (!item) return 0;
	pthread_mutex_lock(&malloc_flag_mutex);
	item->value = (char *)malloc(strlen(value)+1);
	pthread_mutex_unlock(&malloc_flag_mutex);
	strcpy(item->value,value);

	return 1;
}

int ini_set_int(INI *pini,const char *key,int value)
{
	char buffer[VINT_LEN];
	sprintf(buffer,"%d",value);
	return ini_set(pini,key,buffer);
}

int ini_append(INI *pini,const char *key,const char *value)
{
	ini_item *item, *sear, *temp;
	char buffer[LINE_LEN];
	char *k, *v, *s, *p;

	strcpy(buffer,key);
	if (NULL == (p=strchr(buffer,'.'))) {
		if (pini->header)
			strcpy(buffer,pini->header->section);
		else
			strcpy(buffer,DEF_SECT_ROOT);
		strcat(buffer,".");
		strcat(buffer,key);
	} else if (p == buffer) {
		strcpy(buffer,pini->header->section);
		strcat(buffer,key);
	} else if ((unsigned int)(p+1-buffer) == strlen(buffer))
		return 0;
	p = strchr(buffer,'.');
	sear = list_search(pini,buffer);
	if (sear && 0==strcmp(sear->key,p+1))
		return 0;
	pthread_mutex_lock(&malloc_flag_mutex);
	item = (ini_item *)malloc(sizeof(ini_item));
	pthread_mutex_unlock(&malloc_flag_mutex);

	pthread_mutex_lock(&malloc_flag_mutex);
	k = (char *)malloc(strlen(key)+1);
	pthread_mutex_unlock(&malloc_flag_mutex);

	strcpy(k,p+1);
	item->key = k;
	pthread_mutex_lock(&malloc_flag_mutex);
	v = (char *)malloc(strlen(value)+1);
	pthread_mutex_unlock(&malloc_flag_mutex);
	strcpy(v,value);
	item->value = v;

	*(p+1) = 0;
	sear = list_search(pini,buffer);
	if (sear) {
		pthread_mutex_lock(&malloc_flag_mutex);
		s = (char *)malloc(strlen(sear->section)+1);
		pthread_mutex_unlock(&malloc_flag_mutex);
		strcpy(s,sear->section);
		temp = sear->next;
		sear->next = item;
		item->next = temp;
	} else {
		*p = 0;
		pthread_mutex_lock(&malloc_flag_mutex);
		s = (char *)malloc(strlen(buffer)+1);
		pthread_mutex_unlock(&malloc_flag_mutex);

		strcpy(s,buffer);
		item->next = NULL;
		if (pini->header) {
			pini->tailer->next = item;
			pini->tailer = item;
		} else
			pini->header = pini->tailer = item;
	}
	item->section = s;
	pini->length += 1;

	return 1;
}

int ini_append_int(INI *pini,const char *key,int value)
{
	char buffer[VINT_LEN];
	sprintf(buffer,"%d",value);
	return ini_append(pini,key,buffer);
}

int ini_remove(INI *pini,const char *key)
{
	ini_item *item, *temp, *p;

	item = list_search(pini,key);
	if (!item) return 0;
	p = pini->header;
	if (p && !p->next) {
		if (p != item) return 0;
		free(p->section);
		free(p->key);
		free(p->value);
		free(item);
		pini->header = pini->tailer = NULL;
		pini->length -= 1;
		return 1;
	}
	while (p && p->next) {
		if (p->next == item)
			break;
		p = p->next;
	}
	if (!p || !p->next) return 0;
	temp = p->next;
	p->next = temp->next;
	free(temp->section);
	free(temp->key);
	free(temp->value);
	free(temp);
	pini->length -= 1;

	return 1;
}

