/*
 * Copyright (c) 2000 Sun Microsystems, Inc. All Rights Reserved.
 *
 * This software is the confidential and proprietary information of Sun
 * Microsystems, Inc. ("Confidential Information").  You shall not
 * disclose such Confidential Information and shall use it only in
 * accordance with the terms of the license agreement you entered into
 * with Sun.
 *
 * SUN MAKES NO REPRESENTATIONS OR WARRANTIES ABOUT THE SUITABILITY OF THE
 * SOFTWARE, EITHER EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR
 * PURPOSE, OR NON-INFRINGEMENT. SUN SHALL NOT BE LIABLE FOR ANY DAMAGES
 * SUFFERED BY LICENSEE AS A RESULT OF USING, MODIFYING OR DISTRIBUTING
 * THIS SOFTWARE OR ITS DERIVATIVES.
 *
 */

package kdp;

public class Log {

	static int verbose = 0;

	public static void SET_LOG(int level) {
		verbose = level;
	}

	public static void LOG(int level, String s) {
		if (verbose >= level)
			System.out.print(s);
	}

	public static void LOGE(int level, String s) {
		if (verbose == level)
			System.out.print(s);
	}

	public static void LOGN(int level, String s) {
		if (verbose >= level)
			System.out.println(s);
	}

	public static void LOGNE(int level, String s) {
		if (verbose == level)
			System.out.println(s);
	}
}
