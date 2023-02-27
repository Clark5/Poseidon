/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/*
 * Copyright (c) 2007 INRIA, 2008 Timo Bingmann
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation;
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 * Original Author: Mathieu Lacage <mathieu.lacage@sophia.inria.fr>
 * Enhancements: Timo Bingmann <timo.bingmann@student.kit.edu>
 */
#include "gnuplot.h"
#include "ns3/assert.h"
#include <ostream>
#include <stdexcept>

namespace ns3 {

// --- GnuplotDataset::Data ------------------------------------------------ //

struct GnuplotDataset::Data
{
  // *** Data Variables ***

  unsigned int m_references;

  std::string m_title;
  std::string m_extra;

  /**
   * Initializes the reference counter to 1 and sets m_title and m_extra.
   */
  Data(const std::string& title);

  /// Required.
  virtual ~Data();

  /**
   * Returns "plot" or "splot".
   */
  virtual std::string GetCommand () const = 0;

  /**
   * Prints the plot description used as argument to (s)plot. Either the
   * function expression or a datafile description. Should include m_title and
   * m_extra in the output.
   */
  virtual void PrintExpression (std::ostream &os) const = 0;

  /**
   * Print the inline data file contents trailing the plot command. Empty for
   * functions.
   */
  virtual void PrintDatafile (std::ostream &os) const = 0;
};

GnuplotDataset::Data::Data(const std::string& title)
  : m_references (1),
    m_title (title),
    m_extra (m_defaultExtra)
{
}

GnuplotDataset::Data::~Data()
{
}

// --- GnuplotDataset ------------------------------------------------------ //

std::string GnuplotDataset::m_defaultExtra = "";

GnuplotDataset::GnuplotDataset (struct Data* data)
  : m_data (data)
{
}

GnuplotDataset::GnuplotDataset (const GnuplotDataset& original)
  : m_data (original.m_data)
{
  ++m_data->m_references;
}

GnuplotDataset::~GnuplotDataset()
{
  if (--m_data->m_references == 0)
    delete m_data;
}

GnuplotDataset& GnuplotDataset::operator= (const GnuplotDataset& original)
{
  if (this != &original)
    {
      if (--m_data->m_references == 0)
        delete m_data;

      m_data = original.m_data;
      ++m_data->m_references;
    }
  return *this;
}

void
GnuplotDataset::SetTitle (const std::string& title)
{
  m_data->m_title = title;
}

void
GnuplotDataset::SetDefaultExtra (const std::string& extra)
{
  m_defaultExtra = extra;
}
void
GnuplotDataset::SetExtra (const std::string& extra)
{
  m_data->m_extra = extra;
}

// --- Gnuplot2dDataset::Data2d -------------------------------------------- //

struct Gnuplot2dDataset::Data2d : public GnuplotDataset::Data
{
  // *** Data Variables ***

  enum Style  m_style;
  enum ErrorBars m_errorBars;

  PointSet    m_pointset;

  /**
   * Initializes with the values from m_defaultStyle and m_defaultErrorBars.
   */
  Data2d(const std::string& title);

  virtual std::string GetCommand () const;
  virtual void PrintExpression (std::ostream &os) const;
  virtual void PrintDatafile (std::ostream &os) const;
};

Gnuplot2dDataset::Data2d::Data2d(const std::string& title)
  : Data (title),
    m_style (m_defaultStyle),
    m_errorBars (m_defaultErrorBars)
{
}

std::string
Gnuplot2dDataset::Data2d::GetCommand () const
{
  return "plot";
}

void
Gnuplot2dDataset::Data2d::PrintExpression (std::ostream &os) const
{
  os << "'-' ";

  if (m_title.size ())
    os << " title '" << m_title << "'";

  switch (m_style) {
    case LINES:
      os << " with lines";
      break;
    case POINTS:
      switch (m_errorBars)
        {
        case NONE:
          os << " with points";
          break;
        case X:
          os << " with xerrorbars";
          break;
        case Y:
          os << " with yerrorbars";
          break;
        case XY:
          os << " with xyerrorbars";
          break;
        }
      break;
    case LINES_POINTS:
      switch (m_errorBars)
        {
        case NONE:
          os << " with linespoints";
          break;
        case X:
          os << " with errorlines";
          break;
        case Y:
          os << " with yerrorlines";
          break;
        case XY:
          os << " with xyerrorlines";
          break;
        }
      break;
    case DOTS:
      os << " with dots";
      break;
    case IMPULSES:
      os << " with impulses";
      break;
    case STEPS:
      os << " with steps";
      break;
    case FSTEPS:
      os << " with fsteps";
      break;
    case HISTEPS:
      os << " with histeps";
      break;
    }

  if (m_extra.size ())
    os << " " << m_extra;
}

void
Gnuplot2dDataset::Data2d::PrintDatafile (std::ostream &os) const
{
  for (PointSet::const_iterator i = m_pointset.begin ();
       i != m_pointset.end (); ++i)
    {
      if (i->empty) {
          os << std::endl;
          continue;
        }

      switch (m_errorBars) {
        case NONE:
          os << i->x << " " << i->y << std::endl;
          break;
        case X:
          os << i->x << " " << i->y << " " << i->dx << std::endl;
          break;
        case Y:
          os << i->x << " " << i->y << " " << i->dy << std::endl;
          break;
        case XY:
          os << i->x << " " << i->y << " " << i->dx << " " << i->dy << std::endl;
          break;
        }
    }
  os << "e" << std::endl;
}

// --- Gnuplot2dDataset ---------------------------------------------------- //

enum Gnuplot2dDataset::Style Gnuplot2dDataset::m_defaultStyle = LINES;
enum Gnuplot2dDataset::ErrorBars Gnuplot2dDataset::m_defaultErrorBars = NONE;

Gnuplot2dDataset::Gnuplot2dDataset (const std::string& title)
  : GnuplotDataset ( new Data2d (title) )
{
}

void
Gnuplot2dDataset::SetDefaultStyle (enum Style style)
{
  m_defaultStyle = style;
}
void
Gnuplot2dDataset::SetStyle (enum Style style)
{
  reinterpret_cast<Data2d*>(m_data)->m_style = style;
}

void
Gnuplot2dDataset::SetDefaultErrorBars (enum ErrorBars errorBars)
{
  m_defaultErrorBars = errorBars;
}
void
Gnuplot2dDataset::SetErrorBars (enum ErrorBars errorBars)
{
  reinterpret_cast<Data2d*>(m_data)->m_errorBars = errorBars;
}

void 
Gnuplot2dDataset::Add (double x, double y)
{
  NS_ASSERT (reinterpret_cast<Data2d*>(m_data)->m_errorBars == NONE);

  struct Point data;
  data.empty = false;
  data.x = x;
  data.y = y;
  data.dx = 0.0;
  data.dy = 0.0;
  reinterpret_cast<Data2d*>(m_data)->m_pointset.push_back (data);
}

void 
Gnuplot2dDataset::Add (double x, double y, double errorDelta)
{
  NS_ASSERT ( reinterpret_cast<Data2d*>(m_data)->m_errorBars == X ||
              reinterpret_cast<Data2d*>(m_data)->m_errorBars == Y );

  struct Point data;
  data.empty = false;
  data.x = x;
  data.y = y;
  data.dx = errorDelta;
  data.dy = errorDelta;
  reinterpret_cast<Data2d*>(m_data)->m_pointset.push_back (data);
}

void 
Gnuplot2dDataset::Add (double x, double y, double xErrorDelta, double yErrorDelta)
{
  NS_ASSERT ( reinterpret_cast<Data2d*>(m_data)->m_errorBars == XY );

  struct Point data;
  data.empty = false;
  data.x = x;
  data.y = y;
  data.dx = xErrorDelta;
  data.dy = yErrorDelta;
  reinterpret_cast<Data2d*>(m_data)->m_pointset.push_back (data);
}

void
Gnuplot2dDataset::AddEmptyLine ()
{
  struct Point data;
  data.empty = true;
  reinterpret_cast<Data2d*>(m_data)->m_pointset.push_back (data);
}

// --- Gnuplot2dFunction::Function2d --------------------------------------- //

struct Gnuplot2dFunction::Function2d : public GnuplotDataset::Data
{
  // *** Data Variables ***

  std::string m_function;

  /**
   * Initializes with the function and title.
   */
  Function2d(const std::string& title, const std::string& function);

  virtual std::string GetCommand () const;
  virtual void PrintExpression (std::ostream &os) const;
  virtual void PrintDatafile (std::ostream &os) const;
};

Gnuplot2dFunction::Function2d::Function2d(const std::string& title, const std::string& function)
  : Data (title),
    m_function (function)
{
}

std::string
Gnuplot2dFunction::Function2d::GetCommand () const
{
  return "plot";
}

void
Gnuplot2dFunction::Function2d::PrintExpression (std::ostream &os) const
{
  os << m_function;

  if (m_title.size ())
    os << " title '" << m_title << "'";

  if (m_extra.size ())
    os << " " << m_extra;
}

void
Gnuplot2dFunction::Function2d::PrintDatafile (std::ostream &os) const
{
}

// --- Gnuplot2dFunction --------------------------------------------------- //

Gnuplot2dFunction::Gnuplot2dFunction (const std::string& title, const std::string& function)
  : GnuplotDataset ( new Function2d (title, function) )
{
}

void
Gnuplot2dFunction::SetFunction (const std::string& function)
{
  reinterpret_cast<Function2d*>(m_data)->m_function = function;
}

// --- Gnuplot3dDataset::Data3d -------------------------------------------- //

struct Gnuplot3dDataset::Data3d : public GnuplotDataset::Data
{
  // *** Data Variables ***

  std::string   m_style;

  PointSet      m_pointset;

  /**
   * Initializes with value from m_defaultStyle.
   */
  Data3d(const std::string& title);

  virtual std::string GetCommand () const;
  virtual void PrintExpression (std::ostream &os) const;
  virtual void PrintDatafile (std::ostream &os) const;
};

Gnuplot3dDataset::Data3d::Data3d(const std::string& title)
  : Data (title),
    m_style (m_defaultStyle)
{
}

std::string
Gnuplot3dDataset::Data3d::GetCommand () const
{
  return "splot";
}

void
Gnuplot3dDataset::Data3d::PrintExpression (std::ostream &os) const
{
  os << "'-' ";

  if (m_style.size ())
    os << " " << m_style;

  if (m_title.size ())
    os << " title '" << m_title << "'";

  if (m_extra.size ())
    os << " " << m_extra;
}

void
Gnuplot3dDataset::Data3d::PrintDatafile (std::ostream &os) const
{
  for (PointSet::const_iterator i = m_pointset.begin ();
       i != m_pointset.end (); ++i)
    {
      if (i->empty) {
          os << std::endl;
          continue;
        }

      os << i->x << " " << i->y << " " << i->z << std::endl;
    }
  os << "e" << std::endl;
}

// --- Gnuplot3dDataset ---------------------------------------------------- //

std::string Gnuplot3dDataset::m_defaultStyle = "";

Gnuplot3dDataset::Gnuplot3dDataset (const std::string& title)
  : GnuplotDataset ( new Data3d (title) )
{
}

void
Gnuplot3dDataset::SetDefaultStyle (const std::string& style)
{
  m_defaultStyle = style;
}
void
Gnuplot3dDataset::SetStyle (const std::string& style)
{
  reinterpret_cast<Data3d*>(m_data)->m_style = style;
}

void 
Gnuplot3dDataset::Add (double x, double y, double z)
{
  struct Point data;
  data.empty = false;
  data.x = x;
  data.y = y;
  data.z = z;
  reinterpret_cast<Data3d*>(m_data)->m_pointset.push_back (data);
}

void
Gnuplot3dDataset::AddEmptyLine ()
{
  struct Point data;
  data.empty = true;
  reinterpret_cast<Data3d*>(m_data)->m_pointset.push_back (data);
}

// --- Gnuplot3dFunction::Function3d --------------------------------------- //

struct Gnuplot3dFunction::Function3d : public GnuplotDataset::Data
{
  // *** Data Variables ***

  std::string m_function;

  /**
   * Initializes with the function and title.
   */
  Function3d(const std::string& title, const std::string& function);

  virtual std::string GetCommand () const;
  virtual void PrintExpression (std::ostream &os) const;
  virtual void PrintDatafile (std::ostream &os) const;
};

Gnuplot3dFunction::Function3d::Function3d(const std::string& title, const std::string& function)
  : Data (title),
    m_function (function)
{
}

std::string
Gnuplot3dFunction::Function3d::GetCommand () const
{
  return "splot";
}

void
Gnuplot3dFunction::Function3d::PrintExpression (std::ostream &os) const
{
  os << m_function;

  if (m_title.size ())
    os << " title '" << m_title << "'";

  if (m_extra.size ())
    os << " " << m_extra;
}

void
Gnuplot3dFunction::Function3d::PrintDatafile (std::ostream &os) const
{
}

// --- Gnuplot3dFunction --------------------------------------------------- //

Gnuplot3dFunction::Gnuplot3dFunction (const std::string& title, const std::string& function)
  : GnuplotDataset ( new Function3d (title, function) )
{
}

void
Gnuplot3dFunction::SetFunction (const std::string& function)
{
  reinterpret_cast<Function3d*>(m_data)->m_function = function;
}

// ------------------------------------------------------------------------- //

Gnuplot::Gnuplot (const std::string& outputFilename, const std::string& title)
  : m_outputFilename (outputFilename),
    m_terminal ( DetectTerminal (outputFilename) ),
    m_title (title)
{
}

std::string Gnuplot::DetectTerminal (const std::string& filename)
{
  std::string::size_type dotpos = filename.rfind ('.');
  if (dotpos == std::string::npos) return "";

  if (filename.substr (dotpos) == ".png") {
      return "png";
    }
  else if (filename.substr (dotpos) == ".pdf") {
      return "pdf";
    }

  return "";
}

void
Gnuplot::SetTerminal (const std::string& terminal)
{
  m_terminal = terminal;
}

void
Gnuplot::SetTitle (const std::string& title)
{
  m_title = title;
}

void 
Gnuplot::SetLegend (const std::string& xLegend, const std::string& yLegend)
{
  m_xLegend = xLegend;
  m_yLegend = yLegend;
}

void
Gnuplot::SetExtra (const std::string& extra)
{
  m_extra = extra;
}

void
Gnuplot::AppendExtra (const std::string& extra)
{
  m_extra += "\n";
  m_extra += extra;
}

void 
Gnuplot::AddDataset (const GnuplotDataset& dataset)
{
  m_datasets.push_back (dataset);
}

void
Gnuplot::GenerateOutput (std::ostream &os) const
{
  if (m_terminal.size ())
    os << "set terminal " << m_terminal << std::endl;

  if (m_outputFilename.size ())
    os << "set output '" << m_outputFilename << "'" << std::endl;

  if (m_title.size ())
    os << "set title '" << m_title << "'" << std::endl;

  if (m_xLegend.size ())
    os << "set xlabel '" << m_xLegend << "'" << std::endl;

  if (m_yLegend.size ())
    os << "set ylabel '" << m_yLegend << "'" << std::endl;

  if (m_extra.size ())
    os << m_extra << std::endl;

  if (m_datasets.empty ())
    return;

  // Determine the GetCommand() values of all datasets included. Check that all
  // are equal and print the command.

  std::string command = m_datasets.begin ()->m_data->GetCommand ();

  for (Datasets::const_iterator i = m_datasets.begin () + 1;
       i != m_datasets.end (); ++i)
    {
      NS_ASSERT_MSG (command == i->m_data->GetCommand (),
                     "Cannot mix 'plot' and 'splot' GnuplotDatasets.");
    }

  os << command << " ";

  // Print all dataset expressions

  for (Datasets::const_iterator i = m_datasets.begin (); i != m_datasets.end ();)
    {
      i->m_data->PrintExpression (os);

      i++;

      if (i != m_datasets.end ())
        {
          os << ", ";
        }
    }
  os << std::endl;

  // followed by the inline datafile.

  for (Datasets::const_iterator i = m_datasets.begin (); i != m_datasets.end (); i++)
    {
      i->m_data->PrintDatafile (os);
    }
}

// ------------------------------------------------------------------------- //

GnuplotCollection::GnuplotCollection (const std::string& outputFilename)
  : m_outputFilename (outputFilename),
    m_terminal ( Gnuplot::DetectTerminal (outputFilename) )
{
}

void
GnuplotCollection::SetTerminal (const std::string& terminal)
{
  m_terminal = terminal;
}

void
GnuplotCollection::AddPlot (const Gnuplot& plot)
{
  m_plots.push_back (plot);
}

Gnuplot&
GnuplotCollection::GetPlot (unsigned int id)
{
  if (id >= m_plots.size ())
    throw(std::range_error ("Gnuplot id is out of range"));
  else
    return m_plots[id];
}

void
GnuplotCollection::GenerateOutput (std::ostream &os) const
{
  if (m_terminal.size ())
    os << "set terminal " << m_terminal << std::endl;

  if (m_outputFilename.size ())
    os << "set output '" << m_outputFilename << "'" << std::endl;

  for (Plots::const_iterator i = m_plots.begin (); i != m_plots.end (); ++i)
    {
      i->GenerateOutput (os);
    }
}

// ------------------------------------------------------------------------- //

} // namespace ns3
