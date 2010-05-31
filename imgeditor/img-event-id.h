/*
 * img-event-id.h
 *
 *  Created on: 2009-5-26
 *      Author: dongjunjie
 */

#ifndef IMGEVENTID_H_
#define IMGEVENTID_H_

#define IMG_EVENT 0x10000000

#define IMGRES_LISTPANEL  (IMG_EVENT|0x1000)
#define IMAGE_VIEW        (IMG_EVENT|0x2000)
#define DIR_IMAGE_VIEW    (IMAGE_VIEW|0x0100)
#define RES_IMAGE_VIEW    (IMAGE_VIEW|0x0200)
#define DIR_TREE_PANEL    (IMG_EVENT|0x4000)

#define IMGRES_LISTPANEL_ADD_IMAGE  (IMGRES_LISTPANEL|1)
//id = param1

#define IMGRES_LISTPANEL_SEL_CHANGE (IMGRES_LISTPANEL|2)
//id = param1



#define IMAGE_VIEW_SEL_CHANGED  (IMAGE_VIEW|1)
//file_name = param1
//add_data = param2

#define IMAGE_VIEW_REMOVED      (IMAGE_VIEW|2)
//file_name = param1
//id = param2



#define DIR_TREE_PANEL_DIR_CHANGED       (DIR_TREE_PANEL|1)
//dir_path = param1


#endif /* IMGEVENTID_H_ */
