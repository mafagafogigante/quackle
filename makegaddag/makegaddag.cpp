/*
 *  Quackle -- Crossword game artificial intelligence and analysis tool
 *  Copyright (C) 2005-2014 Jason Katz-Brown and John O'Laughlin.
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program. If not, see <http://www.gnu.org/licenses/>.
 */

#include "alphabetparameters.h"

#include <stdio.h>
#include <string>
#include <fstream>
#include <iostream>
#include <iomanip>
#include <vector>
#include <map>
#include <algorithm>

#include <QtCore>

#include "quackleio/froggetopt.h"
#include "quackleio/gaddagfactory.h"
#include "quackleio/util.h"

using namespace std;

int main(int argc, char **argv) {
	QCoreApplication a(argc, argv);

	GetOpt opts;
	QString alphabet;
	QString inputFilename;
	QString outputFilename;
	QString scoringOutputFilename;
	QString versionString;
	
	opts.addOption('f', "input", &inputFilename);
	opts.addOption('o', "output", &outputFilename);
	opts.addOption('o', "scoring", &scoringOutputFilename);
	opts.addOption('a', "alphabet", &alphabet);
	opts.addOption('v', "version", &versionString);

	if (!opts.parse())
		return 1;

	if (alphabet.isNull())
		alphabet = "english";

	if (inputFilename.isNull())
		inputFilename = "gaddaginput.raw";

	if (outputFilename.isNull())
		outputFilename = "output.gaddag";

	if (scoringOutputFilename.isNull())
		scoringOutputFilename = "scoring.gaddag";

	int version = 2;
	if (!versionString.isNull()) {
		version = versionString.toInt();
	}
	
	QString alphabetFile = QString("../data/alphabets/%1.quackle_alphabet").arg(alphabet);
	UVcout << "Using alphabet file: " << QuackleIO::Util::qstringToString(alphabetFile) << endl;
	GaddagFactory factory(QuackleIO::Util::qstringToString(alphabetFile));

	QFile file(inputFilename);
	if (!file.exists())
	{
		UVcout << "Input dictionary does not exist: " << QuackleIO::Util::qstringToString(inputFilename) << endl;
		return false;
	}

	if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
	{
		UVcout << "Could not open " << QuackleIO::Util::qstringToString(inputFilename) << endl;
		return false;
	}

	QTextStream stream(&file);
	stream.setCodec(QTextCodec::codecForName("UTF-8"));

	while (!stream.atEnd())
	{
		QString originalQString;
		stream >> originalQString;

		if (stream.atEnd())
			break;

		const UVString uvString = QuackleIO::Util::qstringToString(originalQString);
		if (!factory.pushWord(uvString) || !factory.addScoringPatterns(uvString)) {
			UVcout << "not encodable without leftover: "
						 << QuackleIO::Util::qstringToString(originalQString) << endl;
		}
	}

	UVcout << "Gaddagizing " << factory.scoringPatternCount() << " scoring patterns..." << endl;
	factory.gaddagizeScoringPatterns();
	
	UVcout << "Sorting " << factory.gaddagizedScoringPatternCount() << " scoring patterns..." << endl;
	factory.sortGaddagizedScoringPatterns();
	
	UVcout << "Sorting " << factory.wordCount() << " word patterns..." << endl;
	factory.sortWords();

	UVcout << "Generating nodes..." << endl;
	factory.generate();
  factory.generateScoring();
	
	UVcout << "Writing indices..." << endl;
	factory.writeIndices(outputFilename.toUtf8().constData(),
											 scoringOutputFilename.toUtf8().constData(),
											 version);

	UVcout << "Wrote " << factory.encodableWords()
				 << " words to " << QuackleIO::Util::qstringToString(outputFilename)
				 << "." << endl;

	UVcout << "Hash: " << QString(QByteArray(factory.hashBytes(), 16).toHex()).toStdString() << endl;

	if (factory.unencodableWords() > 0)
		UVcout << "There were " << factory.unencodableWords() << " words left out." << endl;

	return 0;
}
